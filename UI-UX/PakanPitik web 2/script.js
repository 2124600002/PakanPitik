// Inisialisasi awal
let totalData = 1; 
let stokPakan = 45; // Simulasi stok pakan awal
updateTotalData();

// Fungsi: Eksekusi Manual
function eksekusiManual() {
    alert("Perintah #FEED dikirim ke ESP32!");
    
    // Simulasi pakan berkurang
    stokPakan = Math.max(0, stokPakan - 5); 
    updateStok(stokPakan);

    // Tambah ke tabel history
    tambahHistory("Manual (GUI Web)");
}

// Fungsi: Kirim Jadwal
function kirimJadwal() {
    const pagi = document.getElementById('jadwalPagi').value;
    const siang = document.getElementById('jadwalSiang').value;
    const sore = document.getElementById('jadwalSore').value;
    
    if(!pagi || !siang || !sore) {
        alert("Harap isi semua jam jadwal!");
        return;
    }
    
    // Simulasi pengiriman data
    const payload = `#SET:${pagi},${siang},${sore}`;
    alert(`Jadwal berhasil dikirim ke server:\n${payload}`);
}

// Fungsi: Kirim Durasi
function kirimDurasi() {
    const durasi = document.getElementById('durasiPakan').value;
    
    if(durasi < 500) {
        alert("Durasi minimal adalah 500 ms (0.5 detik)!");
        return;
    }

    const payload = `#DUR:${durasi}`;
    alert(`Durasi pakan baru berhasil dikirim:\n${payload}`);
}

// Fungsi Internal: Update Level Progress Bar
function updateStok(percent) {
    document.getElementById('stokBar').style.width = percent + "%";
    document.getElementById('stokText').innerText = percent + " %";
}

// Fungsi Internal: Menambahkan baris ke tabel History
function tambahHistory(metode) {
    const tbody = document.getElementById('historyTableBody');
    const now = new Date();
    
    // Format YYYY-MM-DD HH:MM:SS
    const waktuStr = now.getFullYear() + "-" + 
                     String(now.getMonth() + 1).padStart(2, '0') + "-" + 
                     String(now.getDate()).padStart(2, '0') + " " + 
                     String(now.getHours()).padStart(2, '0') + ":" + 
                     String(now.getMinutes()).padStart(2, '0') + ":" + 
                     String(now.getSeconds()).padStart(2, '0');

    // Buat elemen tr (baris) baru
    const tr = document.createElement('tr');
    tr.innerHTML = `
        <td>${waktuStr}</td>
        <td>${metode}</td>
        <td>${stokPakan} %</td>
    `;

    // Sisipkan di baris paling atas
    tbody.insertBefore(tr, tbody.firstChild);

    // Update kotak "Last Feed"
    document.getElementById('lastFeedText').innerText = 
        String(now.getHours()).padStart(2, '0') + ":" + String(now.getMinutes()).padStart(2, '0');

    // Update Total Data counter
    totalData++;
    updateTotalData();
}

// Fungsi Internal: Update Counter Total Data
function updateTotalData() {
    document.getElementById('totalDataText').innerText = `Total data: ${totalData}`;
}