<div align="center">
  <img src="Assets\Logo PakanPitik.png" width="250">
  <h1>PakanPitik</h1>
  <p><em>"Inovasi cerdas untuk meringankan beban peternak ayam lokal dan mewujudkan kemandirian pangan"</em></p>
</div>

<hr>

# Deskripsi Proyek
<div align="justify">
Peternak ayam skala lokal seringkali dihadapkan pada tantangan rutinitas yang menguras waktu dan tenaga. Keterlambatan atau ketidakteraturan jadwal pakan dapat menyebabkan tingkat stres yang tinggi dan pertumbuhan unggas yang tidak merata. Di sisi lain, pemberian pakan secara manual seringkali tidak tertakar dengan baik, berujung pada pakan yang tercecer dan terbuang sia-sia (<em>food waste</em>). Hal ini tentu membebani biaya operasional peternak kecil yang sedang berjuang untuk mencapai kemandirian ekonomi.

Hadir sebagai solusi atas permasalahan tersebut, <strong>PakanPitik</strong> adalah sistem pemberi pakan ayam otomatis berbasis <em>Internet of Things</em> (IoT) yang dirancang aplikatif untuk meringankan beban peternak. PakanPitik mengeliminasi keharusan peternak untuk terus-menerus berada di kandang. Melalui integrasi <em>dashboard</em> cerdas, peternak dapat memantau sisa stok pakan secara <em>real-time</em> dari mana saja, dan sistem akan secara otomatis mendistribusikan pakan secara presisi sesuai jadwal. Dengan adanya fitur penyesuaian durasi pakan yang dinamis, takaran pakan dapat diatur agar selalu pas sesuai umur dan populasi ayam, sehingga menekan kerugian finansial akibat pakan yang terbuang.

Secara teknis, sistem ini menggunakan arsitektur <em>Single Source of Truth</em> (SSOT) di mana <strong>ATmega2560</strong> (diprogram dengan <em>Pure Bare-Metal C++</em>) bertindak sebagai <em>Master Brain</em> yang menangani seluruh logika aktuator dan kepastian waktu secara presisi tanpa <em>delay blocking</em>. Sementara itu, <strong>ESP32</strong> ditugaskan khusus sebagai jembatan jaringan (<em>WiFi Bridge</em>) untuk menghubungkan perangkat fisik di kandang dengan Cloud Web Dashboard.

Untuk mendukung sistem otomatisasi dan monitoring, digunakan beberapa komponen utama:
<ul>
  <li><strong>Ultrasonic Sensor HC-SR04</strong> → Mendeteksi jarak dan menghitung sisa ketersediaan (persentase) pakan dalam wadah penampung (hopper).</li>
  <li><strong>RTC DS3231</strong> → Modul <em>Real-Time Clock</em> untuk menjaga keakuratan jam pemicu jadwal pakan meskipun sistem sempat dimatikan atau putus koneksi internet.</li>
  <li><strong>Tombol Pakan Manual</strong> → Mengizinkan peternak untuk mengeksekusi pakan secara fisik langsung dari kandang (Active LOW) saat melakukan inspeksi.</li>
  <li><strong>Motor Servo MG90</strong> → Digerakkan oleh <em>Hardware Timer Fast PWM</em> untuk membuka dan menutup katup saluran pakan secara akurat.</li>
  <li><strong>LCD 16x2 (I2C)</strong> → Menampilkan informasi jam sistem, sisa stok pakan (%), dan status eksekusi secara lokal di kandang.</li>
  <li><strong>ATmega2560</strong> → Mikrokontroler AVR utama pusat kendali (<em>Master</em>).</li>
  <li><strong>ESP32</strong> → Penghubung internet via WiFi untuk sinkronisasi data ke <em>database</em> jarak jauh (<em>Slave/Bridge</em>).</li>
</ul>

Melalui sentuhan otomatisasi tepat guna ini, PakanPitik tidak hanya meningkatkan efisiensi waktu dan tenaga, tetapi juga memberdayakan peternak lokal agar dapat mengelola peternakannya dengan lebih modern, produktif, dan sejahtera.
</div>

# Tujuan Proyek
<div align="justify">
Merancang dan mengimplementasikan sistem pemberian pakan unggas otomatis berbasis IoT untuk memantau ketersediaan pakan secara <em>real-time</em> dan mengontrol aktuator distribusi pakan (berbasis jadwal atau <em>remote on-demand</em>). Proyek ini ditujukan untuk mempermudah pekerjaan peternak, mencegah pakan terbuang percuma melalui fitur durasi dinamis, dan menaikkan efisiensi operasional kandang.
</div>

### Support BY :
>- **Dosen Pengampu** : Akhmad Hendriawan ST., MT.
>- **Mata kuliah** : Mikrokontroller
>- **Program Studi** : D4 Teknik Elektronika A
>- **Politeknik Elektronika Negeri Surabaya**

<hr>

# Anggota Kelompok 1

<div align="center">
  <img src="Assets\Banner kelompok.png" width="400">
</div>
<br>

|      NRP      |        Nama        |        Jobdesk          |
| :-----------: |:----------------:  | :-------------------:   |
| 2124600002    | M. Rizky Ramadhan  | Project Manager         |
| 2124600005    | Musa Hamonangan L. | UI UX Design            |
| 2124600007    | Muh Yusni Muzaky F.| Hardware                |
| 2124600012    | Ferris Adiwidya H. | Software & Documentation|
| 2124600016    | Reyhan Fachri A.H. | Software & Documentation|
| 2124600020    | M Gustav Ardhito   | Hardware                |

# Komponen Hardware

<div align="justify">
<ol>
  <li>Mikrokontroler ATmega2560 (Master Controller)</li>
  <li>Modul ESP32 WiFi (Bridge Controller)</li>
  <li>Modul Real-Time Clock (RTC) DS3231</li>
  <li>Sensor Jarak Ultrasonik HC-SR04</li>
  <li>Motor Servo DC MG90/MG996R</li>
  <li>Modul Display LCD 16x2 + I2C Backpack (PCF8574)</li>
  <li>Push Button (Tactile Switch)</li>
  <li>Modul Step-Down DC-DC LM2596 (12V ke 5V)</li>
  <li>Logic Level Converter Bi-Directional (Jembatan UART 5V ↔ 3.3V)</li>
  <li>Power Supply 12V DC</li>
</ol>
</div>

<hr>

# Visualisasi Sistem

### Desain Hardware 
<div align="center">
  <img src="Hardware\3D PCB.jpeg" width="400">
  <p align="justify">
    <em>Tampilan 3D dari board slave untuk sistem PakanPitik.</em>
  </p>
  <br>
  
  <img src="Hardware\Desain PCB.jpeg" width="400">
  <p align="justify">
    <em>Wiring pada board PCB slave dengan menggunakan bottom layer saja untuk menghemat dan mempermudah dalam proses pencetakkan.</em>
  </p>
  <br>

  <img src="Hardware\Skematik PCB.jpeg" width="400">
  <p align="justify">
    <em>Desain skematik dari board PCB slave yang digunakan pada PakanPitik.</em>
  </p>
</div>

### Desain 3D
<div align="center">
  <img src="3D Design\PakanPitik 3D.bmp" width="400">
  <br><br>
  <img src="3D Design\PakanPitik 3D 2.bmp" width="400">
  <br><br>
  <img src="3D Design\PakanPitik 3D 3.bmp" width="400">
  <br><br>
  <img src="3D Design\PakanPitik 3D 7.bmp" width="400">
  <br><br>
  <img src="3D Design\PakanPitik 3D 5.bmp" width="400">
  <br><br>
  <img src="3D Design\PakanPitik 3D 6.bmp" width="400">
</div>

### Blok Diagram
<div align="center">
  <img src="Blokdiagram\Blockdiagram baru.png" width="400">
</div>

### Flowchart
<div align="center">
  <img src="Flowchart\Flowchart baru 1.png" width="400">
  <br><br>
  <img src="Flowchart\Flowchart baru 2.png" width="400">
</div>

### UI/UX Web Dashboard
<div align="center">
  <img src="UI-UX\Desain UI-UX figma 2.png" width="400">
</div>

### Simulasi Program Wokwi
<div align="justify">
<em>Adaptasi simulasi untuk pengujian sistem PakanPitik yang disesuaikan dengan komponen pada Shield PCB.</em>
</div>

<div align="center">
  <br>
  <a href="https://wokwi.com/projects/465155487486760961" target="_blank">
    <img src="https://img.shields.io/badge/▶_Coba_Simulasi_di_Wokwi-1199EE?style=for-the-badge" alt="Buka Wokwi">
  </a>
  <br><br>
  <img src="Simulasi\Wokwi.png" width="400">
</div>

<hr>

# Cara Kerja Sistem
<div align="justify">
<ol>
  <li><strong>Inisialisasi & Pembacaan Data:</strong> Saat dihidupkan, ATmega2560 menarik data waktu <em>real-time</em> (Jam, Menit, Detik) dari RTC DS3231 via I2C, serta mengukur pantulan frekuensi dari HC-SR04 untuk dikalkulasi menjadi persentase stok pakan (0-100%).</li>
  <li><strong>Sinkronisasi Tampilan & Cloud:</strong> Data waktu dan stok ditampilkan ke layar LCD 16x2. Secara bersamaan, ATmega2560 merangkai string data dan mengirimkannya melalui komunikasi Serial (UART1) ke ESP32. ESP32 kemudian menembakkan data tersebut ke Web Dashboard via koneksi WiFi.</li>
  <li><strong>Logika Pemicu (Triggers):</strong> Sistem terus bersiaga memeriksa 3 kondisi pemicu:
    <ul>
      <li><em>Otomatis:</em> Apakah waktu saat ini sama persis dengan jadwal memori? (Dilengkapi <em>debouncing</em> agar tidak terjadi <em>log</em> ganda di detik yang sama).</li>
      <li><em>Fisik:</em> Apakah tombol manual pada kandang ditekan?</li>
      <li><em>Remote:</em> Apakah ada perintah eksekusi pakan masuk dari Web Dashboard melalui ESP32?</li>
    </ul>
  </li>
  <li><strong>Eksekusi Aktuator Dinamis:</strong> Jika salah satu pemicu aktif, ATmega2560 akan memancarkan sinyal <em>Fast PWM</em> (Hardware Timer) untuk membuka katup motor Servo MG90. Lama katup terbuka bergantung pada variabel <strong>Durasi Pakan Dinamis</strong> yang bisa diatur peternak langsung dari Web Dashboard (untuk mencegah pakan terbuang).</li>
  <li><strong>Perekaman Log (History):</strong> Sebelum pakan turun, ATmega2560 secara mutlak mengirimkan paket data <em>Event</em> (<code>EVENT:AUTO</code>, <code>EVENT:PHYSICAL</code>, atau <code>EVENT:GUI</code>) ke Web Dashboard agar waktu dan alasan pakan keluar tercatat rapi di tabel <em>History</em>.</li>
</ol>
</div>

# Panduan Penggunaan
<div align="justify">
<ol>
  <li><strong>Persiapan Daya:</strong> Hubungkan alat ke sumber listrik PLN pada Power Supply 12V nya. Sistem manajemen daya internal (LM2596) akan menstabilkan tegangan operasi komponen ke 5V.</li>
  <li><strong>Koneksi Jaringan:</strong> Pastikan alat menyala dalam jangkauan sinyal WiFi kandang agar ESP32 dapat secara otomatis merutekan sistem ATmega2560 ke <em>database</em> Cloud.</li>
  <li><strong>Akses Dashboard:</strong> Buka <em>browser</em> di <em>smartphone</em> atau PC, lalu akses halaman Web Dashboard PakanPitik milik Anda.</li>
  <li><strong>Pemantauan:</strong> Pada layar Web (atau LCD lokal), Anda dapat melihat jam operasional alat saat ini dan persentase (%) ketersediaan pakan di dalam tangki hopper.</li>
  <li><strong>Pengaturan Jadwal & Durasi:</strong> Pada panel kendali Web, atur 3 jadwal makan utama (Pagi, Siang, Sore) berformat HH:MM. Masukkan juga <strong>Durasi Pakan</strong> dalam milidetik (contoh: isi 3000 untuk buka katup selama 3 detik) untuk menyesuaikan takaran makanan unggas. Tekan "Kirim" atau "Set".</li>
  <li><strong>Eksekusi Darurat:</strong> Jika ingin memberi pakan instan di luar jadwal, tekan tombol fisik pada alat, atau klik tombol <strong>"Beri Makan Manual"</strong> langsung dari Web Dashboard Anda. Log eksekusi akan langsung tercatat pada tabel riwayat <em>website</em>.</li>
</ol>
</div>