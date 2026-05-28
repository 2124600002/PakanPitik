# README_ALURGIT.md

# 📌 Alur Kerja GitHub Project PakanPitik

Dokumen ini digunakan sebagai panduan kerja seluruh anggota tim dalam menggunakan Git & GitHub pada project **PakanPitik** agar workflow tetap rapi, aman, dan terstruktur.

---

# 👨‍💻 Struktur Tim

| Role    | Tugas                                                   |
| ------- | ------------------------------------------------------- |
| Ketua   | Review Pull Request, merge ke main, mengatur repository |
| Anggota | Mengerjakan fitur pada branch masing-masing             |

---

# 🔒 Aturan Penting

## ❌ DILARANG

* Push langsung ke branch `main`
* Mengubah branch anggota lain
* Menghapus file project tanpa izin
* Force push ke `main`
* Commit file `.pio` atau `.vscode`

---

## ✅ WAJIB

* Kerja di branch masing-masing
* Pull update terbaru sebelum coding
* Commit dengan pesan yang jelas
* Membuat Pull Request setelah selesai bekerja

---

# 🌳 Struktur Branch

| Branch   | Fungsi               |
| -------- | -------------------- |
| `main`   | Branch utama project |
| `dito`   | Branch kerja Dito    |
| `ferris` | Branch kerja Ferris  |
| `musa`   | Branch kerja Musa    |
| `reyhan` | Branch kerja Reyhan  |
| `yusni`  | Branch kerja Yusni   |

---

# 🚀 LANGKAH AWAL SETELAH DIINVITE

---

## 1. Clone Repository

Buka terminal VS Code lalu jalankan:

```bash
git clone git@github.com:2124600002/PakanPitik.git
```

---

## 2. Masuk ke Folder Project

```bash
cd PakanPitik
```

---

## 3. Checkout ke Branch Masing-Masing

Contoh:

```bash
git checkout ferris
```

Ganti `ferris` sesuai nama branch masing-masing anggota.

---

# 🔄 ALUR KERJA HARIAN

Lakukan langkah berikut setiap sebelum dan sesudah bekerja.

---

# 📥 SEBELUM MULAI CODING

## 1. Pastikan berada di branch sendiri

```bash
git branch
```

Jika belum:

```bash
git checkout namabranch
```

---

## 2. Ambil Update Terbaru dari Main

```bash
git pull origin main
```

Tujuan:

* mendapatkan update terbaru dari ketua
* menghindari conflict
* sinkronisasi project

---

# 💻 SETELAH SELESAI CODING

## 1. Tambahkan perubahan

```bash
git add .
```

---

## 2. Commit perubahan

```bash
git commit -m "menambahkan fitur monitoring sensor"
```

Gunakan pesan commit yang jelas dan profesional.

Contoh commit yang baik:

✅

```text
menambahkan pembacaan sensor suhu
memperbaiki bug tombol manual
menambahkan dashboard monitoring
```

❌

```text
fix
baru
update
hehe
```

---

## 3. Push ke Branch Masing-Masing

```bash
git push
```

---

# 🔀 MEMBUAT PULL REQUEST

Setelah push berhasil:

1. Buka repository GitHub
2. Klik tab `Pull Requests`
3. Klik `New Pull Request`
4. Pilih:

   * Base: `main`
   * Compare: `branch masing-masing`
5. Klik `Create Pull Request`

Tambahkan deskripsi perubahan yang dilakukan.

---

# 👑 ALUR REVIEW KETUA

Ketua akan:

1. Review perubahan pada Pull Request
2. Mengecek file yang diubah
3. Memberikan revisi jika diperlukan
4. Approve Pull Request
5. Merge ke branch `main`

---

# 🔄 SETELAH PULL REQUEST DI-MERGE

Semua anggota wajib update branch masing-masing:

```bash
git pull origin main
```

Agar branch tetap sinkron dengan project terbaru.

---

# 🛠 COMMAND PENTING

| Command                 | Fungsi                             |
| ----------------------- | ---------------------------------- |
| `git status`            | Melihat perubahan file             |
| `git branch`            | Melihat branch aktif               |
| `git checkout nama`     | Pindah branch                      |
| `git add .`             | Menambahkan semua perubahan        |
| `git commit -m "pesan"` | Menyimpan perubahan                |
| `git push`              | Upload perubahan ke GitHub         |
| `git pull origin main`  | Mengambil update terbaru dari main |
| `git log --oneline`     | Melihat riwayat commit             |

---

# ⚠️ JIKA TERJADI ERROR

## Repository belum update

```bash
git pull origin main
```

---

## Conflict saat merge

Segera hubungi ketua project sebelum melanjutkan.

---

## Salah branch

Pindah branch:

```bash
git checkout namabranch
```

---

# 📌 Catatan Tambahan

* Branch `main` adalah branch paling penting.
* Semua perubahan wajib melalui Pull Request.
* Jangan commit file yang tidak diperlukan.
* Biasakan commit secara rutin.
* Pull update terbaru sebelum coding.

---

# ✅ Workflow Singkat

```text
Pull update terbaru
↓
Coding di branch sendiri
↓
git add .
↓
git commit
↓
git push
↓
Buat Pull Request
↓
Ketua review
↓
Merge ke main
↓
Semua anggota pull update terbaru
```

---

# 🚀 PakanPitik Team Workflow

Developed with structured Git collaboration workflow.
