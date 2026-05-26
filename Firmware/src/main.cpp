/*
 * ManganPitik v8.1 - Master Controller Firmware (SSOT Enabled)
 * Architecture: Pure AVR Bare-Metal C++ (ATmega2560)
 * Hardware Connections:
 * - USART0: TCP Bridge Wokwi / USB Laptop (Python HMI Logger)
 * - Timer 1 Fast PWM: Pin 11 (PB5/OC1A) -> Motor Servo MG996R
 * - Timer 3 Polling Timing: Pin D2 (PE4/Trig), Pin D3 (PE5/Echo) -> HC-SR04
 * - TWI (I2C): Pin 20 (SDA), Pin 21 (SCL) -> RTC DS1307/DS3231 & LCD 16x2 Backpack
 * - Digital Input: Pin D4 (PG5) -> Manual Feed Push Button
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

// --- DEFINISI OPERASI SERVO (TIMER 1) ---
#define SERVO_TUTUP 2000  // Duty cycle ~1ms (Katup Tertutup Rapat)
#define SERVO_BUKA  4000  // Duty cycle ~2ms (Katup Terbuka Penuh)

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
    
    // Aktifkan Transmitter, Receiver, dan Interupsi RX Complete
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    // Atur format frame: 8 data-bit, 1 stop bit, tanpa parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void usart0_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0))); // Tunggu buffer transmit kosong
    UDR0 = data;
}

void usart0_print(const char* str) {
    while (*str) {
        usart0_transmit(*str++);
    }
}

// Interupsi USART0 untuk Menangkap Perintah dari GUI Python Laptop
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
    TWSR = 0x00; // Prescaler = 1
    TWBR = 72;   // SCL Frequency = 100 kHz (Spesifikasi standar I2C)
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
    lcd_pulse(0x20); // Pindah ke Mode 4-Bit Data Interface
    
    lcd_command(0x28); // Pengaturan 2 baris, font 5x8
    lcd_command(0x0C); // Display On, Cursor Off
    lcd_command(0x06); // Auto-increment kursor
    lcd_command(0x01); // Bersihkan Layar LCD
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

void rtc_get_time(WaktuSistem* t) {
    twi_start();
    twi_write(0xD0); // Alamat tulis RTC
    twi_write(0x00); // Set pointer memori ke register 0x00 (Detik)
    
    twi_start();     // Repeated Start
    twi_write(0xD1); // Alamat baca RTC
    
    t->detik = bcd_to_dec(twi_read_ack() & 0x7F);
    t->menit = bcd_to_dec(twi_read_ack());
    t->jam   = bcd_to_dec(twi_read_nack());
    
    twi_stop();
}

// =========================================================================
// 5. DRIVER KENDALIAN ACTUATOR SERVO (TIMER 1 FAST PWM MODE 14)
// =========================================================================
void servo_init(void) {
    DDRB |= (1 << PB5); // Konfigurasi Pin 11 (PB5/OC1A) Menjadi Output
    
    // Fast PWM Mode 14 (TOP diatur oleh ICR1)
    // Clear OC1A pada Compare Match, Set OC1A pada BOTTOM (Non-Inverting)
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    // Prescaler 8
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    
    ICR1 = 39999;      // Frekuensi Pulsa Pas 50 Hz (Periode = 20ms)
    OCR1A = SERVO_TUTUP; // Kondisi Awal: Katup Pakan Terkunci Rapat
}

void eksekusi_pakan(uint16_t durasi_ms) {
    OCR1A = SERVO_BUKA; // Putar servo ke posisi buka katup pakan
    
    // Delay non-blocking ringan untuk membiarkan pakan jatuh sesuai durasi
    for (uint16_t i = 0; i < durasi_ms / 10; i++) {
        _delay_ms(10);
    }
    
    OCR1A = SERVO_TUTUP; // Kembalikan servo ke posisi mengunci rapat
}

// =========================================================================
// 6. DRIVER SENSOR ULTRASONIK (HC-SR04 PULSE TIMING VIA TIMER 3)
// =========================================================================
void ultrasonic_init(void) {
    DDRE |= (1 << PE4);   // Pin D2 (PE4/Trig) Menjadi Output Trigger
    DDRE &= ~(1 << PE5);  // Pin D3 (PE5/Echo) Menjadi Input Echo
}

uint8_t ambil_level_pakan(void) {
    PORTE &= ~(1 << PE4);
    _delay_us(2);
    PORTE |= (1 << PE4);  // Kirim isyarat Trig HIGH
    _delay_us(10);
    PORTE &= ~(1 << PE4);  // Kembalikan ke LOW
    
    uint32_t counter = 0;
    // Tunggu hingga pin Echo berubah status menjadi HIGH
    while (!(PINE & (1 << PE5))) {
        if (++counter > 50000) return 0; // Proteksi Timeout jika sensor lepas
    }
    
    TCNT3 = 0;
    TCCR3B = (1 << CS31); // Aktifkan Timer 3, Prescaler 8 (Resolusi 0.5 us per tick)
    
    // Hitung durasi selama pulsa Echo berlogika HIGH
    while (PINE & (1 << PE5)) {
        if (TCNT3 > 60000) break; // Pembatasan jarak maksimal pantulan
    }
    TCCR3B = 0; // Matikan Timer 3
    
    uint32_t ticks = TCNT3;
    uint32_t jarak_cm = (ticks * 0.5) * 0.0343 / 2; // Formula konversi jarak fisika suara
    
    // Pemetaan linear persentase wadah pakan (Tinggi Wadah Maksimal = 40 cm)
    // Ingat, amplitudo diset 25 berdasarkan modul.
    if (jarak_cm > 25) jarak_cm = 25; 
    uint8_t persentase_stok = ((25 - jarak_cm) * 100) / 25;
    
    return persentase_stok;
}

// =========================================================================
// 7. PROGRAM UTAMA (MAIN ROUTINE ENTRY POINT)
// =========================================================================
int main(void) {
    // Inisialisasi Seluruh Subsistem Hardware Tingkat Rendah
    usart0_init(115200);
    twi_init();
    lcd_init();
    servo_init();
    ultrasonic_init();
    
    // Atur Pin D4 (PG5) untuk Push Button Manual Feed (Input dengan Pull-Up Internal)
    DDRG &= ~(1 << PG5);
    PORTG |= (1 << PG5);
    
    sei(); // Aktifkan Sakelar Utama Interupsi Global MCU
    
    WaktuSistem rtc_time;
    char buffer_kirim[64];
    char lcd_line1[17];
    char lcd_line2[17];
    
    usart0_print("[MANGANPITIK v8.1] Bare-Metal ATmega2560 Active.\n");
    
    uint8_t sumber_pakan = 0; // 1: Jadwal, 2: Tombol Fisik, 3: Perintah GUI

    while (1) {
        // 1. Ambil Data Aktual Sensor & Penjadwal Waktu
        rtc_get_time(&rtc_time);
        uint8_t stok_pakan = ambil_level_pakan();
        
        // 2. Transmisi Data Status Mentah Berformat ke Python Laptop via USB (USART0)
        sprintf(buffer_kirim, "TIME:%02d:%02d:%02d|STOK:%d|JADWAL:%02d:%02d\n", 
                rtc_time.jam, rtc_time.menit, rtc_time.detik, stok_pakan, jadwal_jam[0], jadwal_menit[0]);
        usart0_print(buffer_kirim);
        
        // 3. Tampilkan Informasi Utama pada Layar LCD Lokal
        sprintf(lcd_line1, "JAM : %02d:%02d:%02d  ", rtc_time.jam, rtc_time.menit, rtc_time.detik);
        lcd_set_cursor(0, 0);
        lcd_print(lcd_line1);
        
        sprintf(lcd_line2, "STOK: %d%%    OK ", stok_pakan);
        lcd_set_cursor(0, 1);
        lcd_print(lcd_line2);
        
        // 4. Manajemen Logika Pemicu Jadwal Waktu Makan Otomatis
        for(uint8_t i = 0; i < 3; i++) {
            if (rtc_time.jam == jadwal_jam[i] && rtc_time.menit == jadwal_menit[i] && rtc_time.detik == 0) {
                flag_beri_makan = 1;
                sumber_pakan = 1; // Ditrigger oleh Jadwal Otomatis
            }
        }
        
        // 5. Manajemen Logika Input Kendali Tombol Fisik (Active LOW)
        if (!(PING & (1 << PG5))) {
            _delay_ms(20); // Debouncing sakelar mekanik
            if (!(PING & (1 << PG5))) {
                flag_beri_makan = 1;
                sumber_pakan = 2; // Ditrigger oleh Tombol Fisik Kandang
                while (!(PING & (1 << PG5))); // Kunci eksekusi hingga tombol dilepas
            }
        }
        
        // 6. Parsing dan Pengenalan Perintah dari GUI Python Laptop via USB
        if (cmd_ready) {
            if (strncmp((const char*)rx_buffer, "#FEED", 5) == 0) {
                flag_beri_makan = 1;
                sumber_pakan = 3; // Ditrigger oleh GUI Python
            } 
            else if (strncmp((const char*)rx_buffer, "#SET:", 5) == 0) {
                uint8_t j1, m1, j2, m2, j3, m3;
                if (sscanf((const char*)rx_buffer, "#SET:%02hhd:%02hhd,%02hhd:%02hhd,%02hhd:%02hhd", 
                           &j1, &m1, &j2, &m2, &j3, &m3) == 6) {
                    jadwal_jam[0] = j1; jadwal_menit[0] = m1;
                    jadwal_jam[1] = j2; jadwal_menit[1] = m2;
                    jadwal_jam[2] = j3; jadwal_menit[2] = m3;
                    usart0_print("[SYSTEM] 3 Jadwal makan baru diperbarui via GUI Laptop.\n");
                }
            }
            rx_index = 0;
            cmd_ready = 0;
        }
        
        // 7. Pusat Eksekusi Penggerak Motor Servo Dispenser Pakan
        if (flag_beri_makan) {
            // KIRIM KONFIRMASI EVENT KE PYTHON SEBELUM DELAY BLOCKING
            if (sumber_pakan == 1)      usart0_print("EVENT:AUTO\n");
            else if (sumber_pakan == 2) usart0_print("EVENT:PHYSICAL\n");
            else if (sumber_pakan == 3) usart0_print("EVENT:GUI\n");

            lcd_set_cursor(0, 1);
            lcd_print("STATUS: FEEDING ");
            eksekusi_pakan(5000); // Buka katup dispenser selama 5 detik penuh
            
            // Reset state
            flag_beri_makan = 0;   
            sumber_pakan = 0;
        }
        
        _delay_ms(200); // Jeda siklus pembacaan sistem teratur
    }
    return 0;
}