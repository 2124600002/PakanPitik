/*
 * ManganPitik v8.2 - Wokwi to PCB Shield Adaptation (Ide 3)
 * Architecture: Pure AVR Bare-Metal C++ (ATmega2560)
 * Hardware Connections (Sesuai Shield PCB):
 * - USART0: TCP Bridge Wokwi / USB Laptop (Python HMI Logger)
 * - Analog Input (ADC0): Pin A0 (PF0) -> Potensiometer (Simulasi Level Pakan)
 * - Analog Output PWM (Timer 0): Pin 13 (PB7/OC0A) -> LED Indikator Servo
 * - TWI (I2C): Pin 20 (SDA), Pin 21 (SCL) -> RTC DS1307/DS3231 & LCD 16x2 Backpack
 * - Digital Input: Pin D7 (PH4) -> Manual Feed Push Button (Active LOW)
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

// --- DEFINISI DRIVER LCD I2C (PCF8574) ---
#define LCD_ADDR         0x4E  // (0x27 << 1) Alamat TWI Tulis PCF8574
#define LCD_REG_SELECT   0x01  // Register Select (RS Bit)
#define LCD_ENABLE       0x04  // Enable Bit (EN Bit)
#define LCD_BACKLIGHT    0x08  // Backlight Control Bit

// --- BUFFER KOMUNIKASI USART0 ---
#define BUF_SIZE 32
volatile char rx_buffer[BUF_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t cmd_ready = 0;

// --- STRUKTUR DATA WAKTU & SISTEM ---
typedef struct {
    uint8_t jam;
    uint8_t menit;
    uint8_t detik;
} WaktuSistem;

volatile uint8_t flag_beri_makan = 0;
// Jadwal Default: 07:00, 12:00, 16:30
uint8_t jadwal_jam[3] = {7, 12, 16};
uint8_t jadwal_menit[3] = {0, 0, 30};

// =========================================================================
// 1. DRIVER UNIVERSAL ASYNCHRONOUS RECEIVER TRANSMITTER (USART0)
// =========================================================================
void usart0_init(uint32_t baud) {
    uint16_t ubrr_val = (F_CPU / 16 / baud) - 1;
    UBRR0H = (unsigned char)(ubrr_val >> 8);
    UBRR0L = (unsigned char)(ubrr_val);
    
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void usart0_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0))); 
    UDR0 = data;
}

void usart0_print(const char* str) {
    while (*str) {
        usart0_transmit(*str++);
    }
}

ISR(USART0_RX_vect) {
    char c = UDR0;
    if (c == '\n' || c == '\r') {
        if (rx_index > 0) {
            rx_buffer[rx_index] = '\0';
            cmd_ready = 1;
        }
    } else if (rx_index < BUF_SIZE - 1) {
        rx_buffer[rx_index++] = c;
    }
}

// =========================================================================
// 2. DRIVER TWO-WIRE INTERFACE (TWI / I2C) MURNI
// =========================================================================
void twi_init(void) {
    TWSR = 0x00; 
    TWBR = 72;   
    TWCR = (1 << TWEN);
}

void twi_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void twi_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO));
}

void twi_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

uint8_t twi_read_ack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t twi_read_nack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

// =========================================================================
// 3. DRIVER LCD CHARACTER 16x2 LEWAT PCF8574 BACKPACK
// =========================================================================
void lcd_pcf_write(uint8_t data) {
    twi_start();
    twi_write(LCD_ADDR);
    twi_write(data | LCD_BACKLIGHT);
    twi_stop();
}

void lcd_pulse(uint8_t data) {
    lcd_pcf_write(data | LCD_ENABLE);
    _delay_us(2);
    lcd_pcf_write(data & ~LCD_ENABLE);
    _delay_us(50);
}

void lcd_send(uint8_t val, uint8_t mode) {
    uint8_t high_nibble = val & 0xF0;
    uint8_t low_nibble = (val << 4) & 0xF0;
    lcd_pulse(high_nibble | mode);
    lcd_pulse(low_nibble | mode);
}

void lcd_command(uint8_t cmd) { lcd_send(cmd, 0); }
void lcd_data(uint8_t data) { lcd_send(data, LCD_REG_SELECT); }

void lcd_init(void) {
    _delay_ms(50);
    lcd_pulse(0x30);
    _delay_ms(5);
    lcd_pulse(0x30);
    _delay_us(150);
    lcd_pulse(0x30);
    lcd_pulse(0x20); 
    
    lcd_command(0x28); 
    lcd_command(0x0C); 
    lcd_command(0x06); 
    lcd_command(0x01); 
    _delay_ms(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = {0x00, 0x40};
    lcd_command(0x80 | (col + row_offsets[row]));
}

void lcd_print(const char* str) {
    while (*str) lcd_data(*str++);
}

// =========================================================================
// 4. DRIVER REAL-TIME CLOCK (RTC DS1307 / DS3231)
// =========================================================================
uint8_t bcd_to_dec(uint8_t val) { return ((val / 16) * 10) + (val % 16); }

void rtc_get_time(WaktuSistem* t) {
    twi_start();
    twi_write(0xD0); 
    twi_write(0x00); 
    
    twi_start();     
    twi_write(0xD1); 
    
    t->detik = bcd_to_dec(twi_read_ack() & 0x7F);
    t->menit = bcd_to_dec(twi_read_ack());
    t->jam   = bcd_to_dec(twi_read_nack());
    
    twi_stop();
}

// =========================================================================
// 5. DRIVER ADC (UNTUK POTENSIOMETER DI PF0)
// =========================================================================
void adc_init(void) {
    // AREF = AVcc (5V referensi internal)
    ADMUX = (1 << REFS0);
    // Enable ADC, Prescaler 128 (16MHz/128 = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read_channel0(void) {
    // Membaca dari channel ADC0 (PF0)
    ADMUX = (ADMUX & 0xF8) | 0x00; 
    ADCSRA |= (1 << ADSC); // Mulai konversi
    while (ADCSRA & (1 << ADSC)); // Tunggu hingga konversi selesai
    return ADC; // Rentang nilai: 0 - 1023
}

uint8_t ambil_level_pakan_simulasi(void) {
    uint16_t nilai_raw = adc_read_channel0();
    // Konversi linear nilai ADC (0-1023) menjadi persentase (0-100%)
    uint8_t persentase = (uint8_t)(((uint32_t)nilai_raw * 100) / 1023);
    return persentase;
}

// =========================================================================
// 6. DRIVER ANALOG OUTPUT PWM (UNTUK LED DI PB7 / TIMER 0)
// =========================================================================
void led_pwm_init(void) {
    DDRB |= (1 << PB7); // Set Pin 13 (PB7) sebagai Output
    
    // Fast PWM Mode pada Timer 0, output Non-Inverting di OC0A (PB7)
    TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);
    // Prescaler 64 untuk mendapatkan frekuensi PWM yang stabil terlihat mata
    TCCR0B = (1 << CS01) | (1 << CS00);
    
    OCR0A = 0; // Kondisi awal LED mati (Duty Cycle 0%)
}

void simulasi_eksekusi_pakan(uint16_t durasi_ms) {
    OCR0A = 255; // LED menyala terang maksimal (Simulasi Servo Membuka)
    
    // Delay non-blocking ringan untuk membiarkan pakan jatuh sesuai durasi
    for (uint16_t i = 0; i < durasi_ms / 10; i++) {
        _delay_ms(10);
    }
    
    OCR0A = 0; // LED mati kembali (Simulasi Servo Menutup)
}

// =========================================================================
// 7. PROGRAM UTAMA (MAIN ROUTINE ENTRY POINT)
// =========================================================================
int main(void) {
    // Inisialisasi Subsistem
    usart0_init(115200);
    twi_init();
    lcd_init();
    adc_init();      // Inisialisasi Potensiometer (PF0)
    led_pwm_init();  // Inisialisasi LED PWM (PB7)
    
    // Atur Pin D7 (PH4) untuk Push Button Manual Feed (Input dengan Pull-Up Internal)
    DDRH &= ~(1 << PH4);
    PORTH |= (1 << PH4);
    
    sei(); // Aktifkan Interupsi Global
    
    WaktuSistem rtc_time;
    char buffer_kirim[64];
    char lcd_line1[17];
    char lcd_line2[17];
    
    usart0_print("[MANGANPITIK v8.2] Mode Shield PCB: Potentio (PF0) & LED (PB7) Active.\n");
    
    uint8_t sumber_pakan = 0; // 1: Jadwal, 2: Tombol Fisik, 3: Perintah GUI

    while (1) {
        // 1. Ambil Data (RTC & ADC Potensiometer)
        rtc_get_time(&rtc_time);
        uint8_t stok_pakan = ambil_level_pakan_simulasi();
        
        // 2. Transmisi Data ke Python (Format tetap dipertahankan)
        sprintf(buffer_kirim, "TIME:%02d:%02d:%02d|STOK:%d|JADWAL:%02d:%02d\n", 
                rtc_time.jam, rtc_time.menit, rtc_time.detik, stok_pakan, jadwal_jam[0], jadwal_menit[0]);
        usart0_print(buffer_kirim);
        
        // 3. Tampilkan Informasi di LCD
        sprintf(lcd_line1, "JAM : %02d:%02d:%02d  ", rtc_time.jam, rtc_time.menit, rtc_time.detik);
        lcd_set_cursor(0, 0);
        lcd_print(lcd_line1);
        
        sprintf(lcd_line2, "STOK: %d%%    OK ", stok_pakan);
        lcd_set_cursor(0, 1);
        lcd_print(lcd_line2);
        
        // 4. Logika Pemicu Jadwal Otomatis
        for(uint8_t i = 0; i < 3; i++) {
            if (rtc_time.jam == jadwal_jam[i] && rtc_time.menit == jadwal_menit[i] && rtc_time.detik == 0) {
                flag_beri_makan = 1;
                sumber_pakan = 1;
            }
        }
        
        // 5. Logika Pemicu Tombol Fisik (PH4)
        if (!(PINH & (1 << PH4))) {
            _delay_ms(20); 
            if (!(PINH & (1 << PH4))) {
                flag_beri_makan = 1;
                sumber_pakan = 2; 
                while (!(PINH & (1 << PH4))); 
            }
        }
        
        // 6. Logika Pemicu via GUI Python (Serial Command)
        if (cmd_ready) {
            if (strncmp((const char*)rx_buffer, "#FEED", 5) == 0) {
                flag_beri_makan = 1;
                sumber_pakan = 3; 
            } 
            else if (strncmp((const char*)rx_buffer, "#SET:", 5) == 0) {
                uint8_t j1, m1, j2, m2, j3, m3;
                if (sscanf((const char*)rx_buffer, "#SET:%02hhd:%02hhd,%02hhd:%02hhd,%02hhd:%02hhd", 
                           &j1, &m1, &j2, &m2, &j3, &m3) == 6) {
                    jadwal_jam[0] = j1; jadwal_menit[0] = m1;
                    jadwal_jam[1] = j2; jadwal_menit[1] = m2;
                    jadwal_jam[2] = j3; jadwal_menit[2] = m3;
                    usart0_print("[SYSTEM] 3 Jadwal diperbarui.\n");
                }
            }
            rx_index = 0;
            cmd_ready = 0;
        }
        
        // 7. Pusat Eksekusi Aktuator (LED PWM)
        if (flag_beri_makan) {
            if (sumber_pakan == 1)      usart0_print("EVENT:AUTO\n");
            else if (sumber_pakan == 2) usart0_print("EVENT:PHYSICAL\n");
            else if (sumber_pakan == 3) usart0_print("EVENT:GUI\n");

            lcd_set_cursor(0, 1);
            lcd_print("STATUS: FEEDING ");
            
            // Nyalakan LED PB7 selama 5 detik sebagai indikator pemberian pakan
            simulasi_eksekusi_pakan(5000); 
            
            flag_beri_makan = 0;   
            sumber_pakan = 0;
        }
        
        _delay_ms(200); 
    }
    return 0;
}