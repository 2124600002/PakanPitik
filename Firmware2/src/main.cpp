/*
 * ManganPitik v8.3.0 - Full Bare-Metal Sync Edition
 * Architecture: Pure AVR Bare-Metal C++ (ATmega2560)
 * Hardware Connections (Sesuai Shield PCB):
 * - USART0: USB Port Laptop (Python HMI Logger pada 9600 BPS)
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
#include <stdlib.h>

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

// --- Variabel Pencegah Double Log (Debouncing Waktu) ---
uint8_t menit_terakhir_eksekusi = 60; // Nilai awal tidak valid

// Jadwal Default: 07:00, 12:00, 16:30
uint8_t jadwal_jam[3] = {7, 12, 16};
uint8_t jadwal_menit[3] = {0, 0, 30};

// --- VARIABEL DURASI PAKAN (dalam milidetik) ---
uint16_t durasi_pakan_ms = 5000; // Default: 5 detik

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
    if (cmd_ready) return; // Kunci buffer jika perintah belum diproses loop utama

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
    // Penulisan 4-bit Nibble Mode untuk PCF8574
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
uint8_t dec_to_bcd(uint8_t val) { return ((val / 10) * 16) + (val % 10); }

// Fungsi Tulis Waktu Awal ke Modul RTC
void rtc_set_time(uint8_t jam, uint8_t menit, uint8_t detik) {
    twi_start();
    twi_write(0xD0); // Alamat I2C Tulis RTC
    twi_write(0x00); // Set pointer register ke 0x00 (Detik)
    
    twi_write(dec_to_bcd(detik) & 0x7F); // Bit CH (Clock Halt) diset 0 agar RTC berjalan
    twi_write(dec_to_bcd(menit));
    twi_write(dec_to_bcd(jam) & 0x3F);   // Menggunakan mode 24 Jam
    twi_stop();
}

void rtc_get_time(WaktuSistem* t) {
    twi_start();
    twi_write(0xD0); 
    twi_write(0x00); 
    
    twi_start();     
    twi_write(0xD1); // Alamat I2C Baca RTC
    
    t->detik = bcd_to_dec(twi_read_ack() & 0x7F);
    t->menit = bcd_to_dec(twi_read_ack());
    t->jam   = bcd_to_dec(twi_read_nack());
    twi_stop();
}

// =========================================================================
// 5. DRIVER ADC (UNTUK POTENSIOMETER DI PF0)
// =========================================================================
void adc_init(void) {
    ADMUX = (1 << REFS0); // Referensi AVCC dengan kapasitor eksternal pada pin AREF
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
}

uint16_t adc_read_channel0(void) {
    ADMUX = (ADMUX & 0xF8) | 0x00; // Pilih channel ADC0 (PF0)
    ADCSRA |= (1 << ADSC); // Mulai Konversi
    while (ADCSRA & (1 << ADSC)); 
    return ADC; 
}

uint8_t ambil_level_pakan_simulasi(void) {
    uint16_t nilai_raw = adc_read_channel0();
    return (uint8_t)(((uint32_t)nilai_raw * 100) / 1023);
}

// =========================================================================
// 6. DRIVER ANALOG OUTPUT PWM (UNTUK LED DI PB7 / TIMER 0)
// =========================================================================
void led_pwm_init(void) {
    DDRB |= (1 << PB7); 
    TCCR0A = (1 << WGM01) | (1 << WGM00); // Fast PWM Mode
    TCCR0B = (1 << CS01) | (1 << CS00);   // Prescaler 64
    PORTB &= ~(1 << PB7); 
}

void simulasi_eksekusi_pakan(uint16_t durasi_ms) {
    TCCR0A |= (1 << COM0A1); // Hubungkan Timer ke pin PB7
    OCR0A = 255;             // Set kecerahan penuh (100% duty-cycle)
    for (uint16_t i = 0; i < durasi_ms / 10; i++) {
        _delay_ms(10);
    }
    TCCR0A &= ~(1 << COM0A1); // Putuskan pin dari timer
    PORTB &= ~(1 << PB7);     // Matikan pin total
}

// =========================================================================
// 7. PROGRAM UTAMA
// =========================================================================
int main(void) {
    usart0_init(9600); // Set baud rate ke 9600 demi nilai error clock 0%
    twi_init();
    lcd_init();
    adc_init();      
    led_pwm_init();  
    
    DDRH &= ~(1 << PH4); // Pin D7 (PH4) sebagai input tombol
    PORTH |= (1 << PH4);  // Aktifkan internal pull-up
    
    sei(); // Aktifkan Interupsi Global
    
    // -------------------------------------------------------------------------
    // ATUR JAM AWAL DI SINI (Contoh: Jam 14, Menit 30, Detik 0)
    // PERINGATAN: Beri tanda komentar (//) pada baris ini setelah pengunggahan pertama 
    // agar jam di dalam RTC tidak ter-reset kembali setiap kali alat dinyalakan!
    // -------------------------------------------------------------------------
    rtc_set_time(22, 30, 0); 
    
    WaktuSistem rtc_time;
    char buffer_kirim[64];
    char lcd_line1[17];
    char lcd_line2[17];
    
    usart0_print("[MANGANPITIK v8.3.0] Ready - Hardware Sync Active.\n");
    
    uint8_t sumber_pakan = 0; 

    while (1) {
        rtc_get_time(&rtc_time);
        uint8_t stok_pakan = ambil_level_pakan_simulasi();
        
        // Transmisi string data terstruktur menuju HMI Python
        sprintf(buffer_kirim, "TIME:%02d:%02d:%02d|STOK:%d|JADWAL:%02d:%02d\n", 
                rtc_time.jam, rtc_time.menit, rtc_time.detik, stok_pakan, jadwal_jam[0], jadwal_menit[0]);
        usart0_print(buffer_kirim);
        
        // Output Tampilan Layar LCD baris 1
        sprintf(lcd_line1, "JAM : %02d:%02d:%02d  ", rtc_time.jam, rtc_time.menit, rtc_time.detik);
        lcd_set_cursor(0, 0);
        lcd_print(lcd_line1);
        
        // Output Tampilan Layar LCD baris 2
        sprintf(lcd_line2, "STOK: %d%%    OK ", stok_pakan);
        lcd_set_cursor(0, 1);
        lcd_print(lcd_line2);
        
        // Cek Logika Pencocokan Jadwal Otomatis
        for(uint8_t i = 0; i < 3; i++) {
            if (rtc_time.jam == jadwal_jam[i] && 
                rtc_time.menit == jadwal_menit[i] && 
                rtc_time.detik == 0 && 
                rtc_time.menit != menit_terakhir_eksekusi) {
                
                flag_beri_makan = 1;
                sumber_pakan = 1;
                menit_terakhir_eksekusi = rtc_time.menit; 
            }
        }
        
        if (rtc_time.detik > 5) {
             menit_terakhir_eksekusi = 60; 
        }
        
        // Cek Input Tombol Fisik Manual (PH4) dengan software debounce
        if (!(PINH & (1 << PH4))) {
            _delay_ms(20); 
            if (!(PINH & (1 << PH4))) {
                flag_beri_makan = 1;
                sumber_pakan = 2; 
                while (!(PINH & (1 << PH4))); 
            }
        }
        
        // Memproses instruksi Command Serial dari GUI Laptop
        if (cmd_ready) {
            if (strncmp((const char*)rx_buffer, "#FEED", 5) == 0) {
                flag_beri_makan = 1;
                sumber_pakan = 3; 
            } 
            else if (strncmp((const char*)rx_buffer, "#SET:", 5) == 0) {
                int j1, m1, j2, m2, j3, m3;
                if (sscanf((const char*)rx_buffer, "#SET:%d:%d,%d:%d,%d:%d", 
                           &j1, &m1, &j2, &m2, &j3, &m3) == 6) {
                    jadwal_jam[0] = (uint8_t)j1; jadwal_menit[0] = (uint8_t)m1;
                    jadwal_jam[1] = (uint8_t)j2; jadwal_menit[1] = (uint8_t)m2;
                    jadwal_jam[2] = (uint8_t)j3; jadwal_menit[2] = (uint8_t)m3;
                    usart0_print("[SYSTEM] 3 Jadwal diperbarui.\n");
                }
            }
            else if (strncmp((const char*)rx_buffer, "#DUR:", 5) == 0) {
                int durasi_baru = atoi((const char*)rx_buffer + 5);
                if (durasi_baru < 500) durasi_baru = 500;
                durasi_pakan_ms = (uint16_t)durasi_baru; 
                
                char info_durasi[64];
                sprintf(info_durasi, "[SYSTEM] Durasi LED (Pakan) berhasil diset ke: %u ms.\n", durasi_pakan_ms);
                usart0_print(info_durasi);
            }
            
            rx_index = 0;
            cmd_ready = 0;
        }
        
        // Eksekusi Pakan Aktuator
        if (flag_beri_makan) {
            if (sumber_pakan == 1)      usart0_print("EVENT:AUTO\n");
            else if (sumber_pakan == 2) usart0_print("EVENT:PHYSICAL\n");
            else if (sumber_pakan == 3) usart0_print("EVENT:GUI\n");

            lcd_set_cursor(0, 1);
            lcd_print("STATUS: FEEDING ");
            
            simulasi_eksekusi_pakan(durasi_pakan_ms); 
            
            flag_beri_makan = 0;   
            sumber_pakan = 0;
        }
        
        _delay_ms(200); 
    }
    return 0;
}