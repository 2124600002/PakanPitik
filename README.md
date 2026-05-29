# <img src="Assets\Logo PakanPitik.png" width="250">
# PakanPitik

*"Inovasi cerdas untuk meringankan beban peternak ayam lokal dan mewujudkan kemandirian pangan"*

# Deskripsi Proyek

Peternak ayam skala lokal seringkali dihadapkan pada tantangan rutinitas yang menguras waktu dan tenaga. Keterlambatan atau ketidakteraturan jadwal pakan dapat menyebabkan tingkat stres yang tinggi dan pertumbuhan unggas yang tidak merata. Di sisi lain, pemberian pakan secara manual seringkali tidak tertakar dengan baik, berujung pada pakan yang tercecer dan terbuang sia-sia (*food waste*). Hal ini tentu membebani biaya operasional peternak kecil yang sedang berjuang untuk mencapai kemandirian ekonomi.

Hadir sebagai solusi atas permasalahan tersebut, **PakanPitik** adalah sistem pemberi pakan ayam otomatis berbasis *Internet of Things* (IoT) yang dirancang aplikatif untuk meringankan beban peternak. PakanPitik mengeliminasi keharusan peternak untuk terus-menerus berada di kandang. Melalui integrasi *dashboard* cerdas, peternak dapat memantau sisa stok pakan secara *real-time* dari mana saja, dan sistem akan secara otomatis mendistribusikan pakan secara presisi sesuai jadwal. Dengan adanya fitur penyesuaian durasi pakan yang dinamis, takaran pakan dapat diatur agar selalu pas sesuai umur dan populasi ayam, sehingga menekan kerugian finansial akibat pakan yang terbuang.

Secara teknis, sistem ini menggunakan arsitektur *Single Source of Truth* (SSOT) di mana **ATmega2560** (diprogram dengan *Pure Bare-Metal C++*) bertindak sebagai *Master Brain* yang menangani seluruh logika aktuator dan kepastian waktu secara presisi tanpa *delay blocking*. Sementara itu, **ESP32** ditugaskan khusus sebagai jembatan jaringan (*WiFi Bridge*) untuk menghubungkan perangkat fisik di kandang dengan Cloud Web Dashboard.

Untuk mendukung sistem otomatisasi dan monitoring, digunakan beberapa komponen utama:
- **Ultrasonic Sensor HC-SR04** → Mendeteksi jarak dan menghitung sisa ketersediaan (persentase) pakan dalam wadah penampung (hopper).
- **RTC DS3231** → Modul *Real-Time Clock* untuk menjaga keakuratan jam pemicu jadwal pakan meskipun sistem sempat dimatikan atau putus koneksi internet.
- **Tombol Pakan Manual** → Mengizinkan peternak untuk mengeksekusi pakan secara fisik langsung dari kandang (Active LOW) saat melakukan inspeksi.
- **Motor Servo MG90** → Digerakkan oleh *Hardware Timer Fast PWM* untuk membuka dan menutup katup saluran pakan secara akurat.
- **LCD 16x2 (I2C)** → Menampilkan informasi jam sistem, sisa stok pakan (%), dan status eksekusi secara lokal di kandang.
- **ATmega2560** → Mikrokontroler AVR utama pusat kendali (*Master*).
- **ESP32** → Penghubung internet via WiFi untuk sinkronisasi data ke *database* jarak jauh (*Slave/Bridge*).

Melalui sentuhan otomatisasi tepat guna ini, PakanPitik tidak hanya meningkatkan efisiensi waktu dan tenaga, tetapi juga memberdayakan peternak lokal agar dapat mengelola peternakannya dengan lebih modern, produktif, dan sejahtera.

# Tujuan Proyek

Merancang dan mengimplementasikan sistem pemberian pakan unggas otomatis berbasis IoT untuk memantau ketersediaan pakan secara *real-time* dan mengontrol aktuator distribusi pakan (berbasis jadwal atau *remote on-demand*). Proyek ini ditujukan untuk mempermudah pekerjaan peternak, mencegah pakan terbuang percuma melalui fitur durasi dinamis, dan menaikkan efisiensi operasional kandang.

### Support BY :
>- Dosen Pengampu : Akhmad Hendriawan ST., MT.
>- Mata kuliah : Mikrokontroller
>- Program Studi : D4 Teknik Elektronika A
>- Politeknik Elektronika Negeri Surabaya

# Anggota Tim

# <img src="Assets\cover kelompok.png">

|      NRP      |        Nama        |        Jobdesk          |
| :-----------: |:----------------:  | :-------------------:   |
| 2124600002    | M. Rizky Ramadhan  | Project Manager         |
| 2124600005    | Musa Hamonangan L. | UI UX Design            |
| 2124600007    | Muh Yusni Muzaky F.| Hardware                |
| 2124600012    | Ferris Adiwidya H. | Software & Documentation|
| 2124600016    | Reyhan Fachri A.H. | Software & Documentation|
| 2124600020    | Muh. Gustav Ardhito| Hardware                |

# Komponen Hardware

1. Mikrokontroler ATmega2560 (Master Controller)
2. Modul ESP32 WiFi (Bridge Controller)
3. Modul Real-Time Clock (RTC) DS3231
4. Sensor Jarak Ultrasonik HC-SR04
5. Motor Servo DC MG90/MG996R
6. Modul Display LCD 16x2 + I2C Backpack (PCF8574)
7. Push Button (Tactile Switch)
8. Modul Step-Down DC-DC LM2596 (12V ke 5V)
9. Logic Level Converter Bi-Directional (Jembatan UART 5V ↔ 3.3V)
10. Power Supply 12V DC

# Hardware

# <img src="Hardware\Gambar Hardware.png">

# Desain 3D

# <img src="3D\Gambar 3D.png">

# Blok Diagram

# <img src="Blokdiagram\Blockdiagram baru.png">

# Flowchart

*Menampilkan alur logika Single Source of Truth (SSOT) dan manajemen pemicu sistem.*
# <img src="Flowchart\Flowchart baru 1.png" width="80%">

# <img src="Flowchart\Flowchart baru 2.png" width="80%">

# UI/UX Web Dashboard

# <img src="UIUX\Dashboard Web.png">

# Simulasi Program Wokwi (Tugas 3)

*Adaptasi sementara untuk pengujian akademis (Sesuai rubrik Shield PCB)*
# <img src="Simulasi\Wokwi.png">

# Cara Kerja Sistem

1. **Inisialisasi & Pembacaan Data:** Saat dihidupkan, ATmega2560 menarik data waktu *real-time* (Jam, Menit, Detik) dari RTC DS3231 via I2C, serta mengukur pantulan frekuensi dari HC-SR04 untuk dikalkulasi menjadi persentase stok pakan (0-100%).
2. **Sinkronisasi Tampilan & Cloud:** Data waktu dan stok ditampilkan ke layar LCD 16x2. Secara bersamaan, ATmega2560 merangkai string data dan mengirimkannya melalui komunikasi Serial (UART1) ke ESP32. ESP32 kemudian menembakkan data tersebut ke Web Dashboard via koneksi WiFi.
3. **Logika Pemicu (Triggers):** Sistem terus bersiaga memeriksa 3 kondisi pemicu: 
   - *Otomatis:* Apakah waktu saat ini sama persis dengan jadwal memori? (Dilengkapi *debouncing* agar tidak terjadi *log* ganda di detik yang sama).
   - *Fisik:* Apakah tombol manual pada kandang ditekan?
   - *Remote:* Apakah ada perintah eksekusi pakan masuk dari Web Dashboard melalui ESP32?
4. **Eksekusi Aktuator Dinamis:** Jika salah satu pemicu aktif, ATmega2560 akan memancarkan sinyal *Fast PWM* (Hardware Timer) untuk membuka katup motor Servo MG90. Lama katup terbuka bergantung pada variabel **Durasi Pakan Dinamis** yang bisa diatur peternak langsung dari Web Dashboard (untuk mencegah pakan terbuang).
5. **Perekaman Log (History):** Sebelum pakan turun, ATmega2560 secara mutlak mengirimkan paket data *Event* (`EVENT:AUTO`, `EVENT:PHYSICAL`, atau `EVENT:GUI`) ke Web Dashboard agar waktu dan alasan pakan keluar tercatat rapi di tabel *History*.

# Panduan Penggunaan

1. **Persiapan Daya:** Hubungkan alat ke sumber listrik PLN pada Power Supply 12V nya. Sistem manajemen daya internal (LM2596) akan menstabilkan tegangan operasi komponen ke 5V.
2. **Koneksi Jaringan:** Pastikan alat menyala dalam jangkauan sinyal WiFi kandang agar ESP32 dapat secara otomatis merutekan sistem ATmega2560 ke *database* Cloud.
3. **Akses Dashboard:** Buka *browser* di *smartphone* atau PC, lalu akses halaman Web Dashboard PakanPitik milik Anda. 
4. **Pemantauan:** Pada layar Web (atau LCD lokal), Anda dapat melihat jam operasional alat saat ini dan persentase (%) ketersediaan pakan di dalam tangki hopper.
5. **Pengaturan Jadwal & Durasi:** Pada panel kendali Web, atur 3 jadwal makan utama (Pagi, Siang, Sore) berformat HH:MM. Masukkan juga **Durasi Pakan** dalam milidetik (contoh: isi 3000 untuk buka katup selama 3 detik) untuk menyesuaikan takaran makanan unggas. Tekan "Kirim" atau "Set".
6. **Eksekusi Darurat:** Jika ingin memberi pakan instan di luar jadwal, tekan tombol fisik pada alat, atau klik tombol **"Beri Makan Manual"** langsung dari Web Dashboard Anda. Log eksekusi akan langsung tercatat pada tabel riwayat *website*.