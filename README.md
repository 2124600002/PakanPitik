# <img src="">
# Smart Chicken Feeder (SCF)

# Deskripsi Proyek: Smart Pakan Ayam

Smart Chicken Feeder adalah sistem otomatis pemberi pakan ayam berbasis mikrokontroler yang dirancang untuk membantu peternak dalam memonitor dan mengatur pemberian pakan secara real-time. Sistem ini bertujuan meningkatkan efisiensi pemberian pakan, mengurangi pemborosan, dan mempermudah monitoring kondisi kandang.

Sistem menggunakan antarmuka berbasis web/mobile yang dirancang melalui Figma sehingga pengguna dapat memantau kondisi pakan dan jadwal pemberian pakan dengan mudah.

Untuk mendukung sistem monitoring, digunakan beberapa sensor dan aktuator:

- Ultrasonic Sensor → untuk mendeteksi ketersediaan pakan dalam penampung.
- Servo Motor → untuk membuka dan menutup katup pakan otomatis.
- DHT22 → untuk memonitor suhu dan kelembaban kandang ayam.
- NodeMCU / ESP32 → sebagai mikrokontroler utama dan penghubung internet.
- WiFi / MQTT → komunikasi data secara real-time.

Dengan integrasi sensor dan sistem monitoring digital, Smart Chicken Feeder dapat membantu peternak melakukan pemberian pakan secara otomatis dan terjadwal sehingga meningkatkan produktivitas peternakan.

# Tujuan Proyek

Merancang dan mengimplementasikan sistem pemberian pakan ayam otomatis berbasis IoT menggunakan sensor Ultrasonic, DHT22, dan Servo Motor untuk memonitor ketersediaan pakan, kondisi kandang, serta mengontrol distribusi pakan secara real-time guna meningkatkan efisiensi dan produktivitas peternakan ayam.

### Support BY :
>- Dosen Pengampu : Akhmad Hendriawan ST., MT.
>- Mata kuliah : Workshop Mikrokontroller
>- Program Studi : D4 Teknik Elektronika
>- Politeknik Elektronika Negeri Surabaya

# Anggota Tim

# <img src="Assets\cover kelompok.png">

|      NRP      |        Nama        |        Jobdesk        |
| :-----------: |:----------------:  | :-------------------: |
| 2124600002    | M. Rizky Ramadhan  | Hardware Specialist   |
| 2124600005    | Musa Hamonangan L. | UI UX Design          |
| 2124600007    | Muh Yusni Muzaky F.| Project Manager       |
| 2124600012    | Ferris Adiwidya H. | Software              |
| 2124600016    | Reyhan Fachri A.H. | 3D Design             |
| 2124600020    | Muh. Gustav Ardhito| Software              |

# Komponen

>- Ultrasonic HC-SR04 → mendeteksi level pakan.
>- Servo Motor → membuka katup pakan otomatis.
>- ESP32 / NodeMCU → mikrokontroler utama.
>- WiFi / MQTT → komunikasi data IoT.
>- Firebase / MySQL → database monitoring.
>- Web / Mobile App → monitoring dan kontrol sistem.

# Hardware

# <img src="">

# Desain 3D

# <img src="">

# Blok Diagram

# <img src="Blokdiagram\blockdiagram_nyata.png">

# Flowchart

# <img src="Flowchart\flowchart nyata.png" width="50%">
- Sensor membaca kondisi pakan dan lingkungan kandang.
- ESP32 memproses data sensor.
- Data dikirim ke database melalui WiFi.
- Sistem web/mobile menampilkan monitoring secara real-time.
- Servo motor membuka katup pakan sesuai jadwal atau kondisi tertentu.

# UI/UX

# <img src="gambarUI">

### Website Monitoring
- Monitoring stok pakan
- Jadwal pemberian pakan
- Status pemberian pakan otomatis

# Simulasi Program

https://wokwi.com/

# Cara Kerja Sistem

1. Sensor ultrasonic membaca ketersediaan pakan pada penampung.
2. ESP32 memproses data sensor.
3. Jika jadwal pemberian pakan aktif, servo motor membuka katup otomatis.
4. Data monitoring dikirim ke database.
5. User dapat memonitor kondisi melalui website atau aplikasi.

# Panduan Penggunaan

1. Pastikan perangkat terhubung dengan daya.
2. Hubungkan ESP32 ke jaringan WiFi.
3. Akses website monitoring.
4. Atur jadwal pemberian pakan.
5. Sistem akan memberikan pakan secara otomatis.
6. Monitoring kondisi kandang dapat dilakukan secara real-time.
