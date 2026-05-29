import tkinter as tk
from tkinter import ttk
import socket
import threading
import time
from datetime import datetime

# Konfigurasi Wokwi TCP
HOST = '127.0.0.1' 
PORT = 4000

class ManganPitikHMI:
    def __init__(self, root):
        self.root = root
        self.root.title("PanganPitik")
        # --- PERUBAHAN: Lebar di-resize sedikit agar input muat ---
        self.root.geometry("650x650") 
        self.root.configure(padx=15, pady=15)
        
        self.sock = None
        self.is_running = True
        self.last_feed_time = None

        self.setup_ui()
        self.connect_socket()

    def setup_ui(self):
        # Gunakan theme yang lebih rapi
        style = ttk.Style()
        style.theme_use('clam')

        # --- TITLE ---
        title_label = ttk.Label(self.root, text="PANGANPITIK CONTROL CENTER", font=("Arial", 16, "bold"))
        title_label.pack(pady=(0, 15))

        # --- PANEL ATAS: STATUS & CONTROL ---
        top_frame = ttk.Frame(self.root)
        top_frame.pack(fill="x", pady=5)

        # Kiri: Status Live
        status_frame = ttk.LabelFrame(top_frame, text=" Live Status (Wokwi) ", padding=(10, 10))
        status_frame.pack(side="left", fill="both", expand=True, padx=(0, 5))

        self.lbl_time = ttk.Label(status_frame, text="Jam MCU : --:--:--", font=("Arial", 11))
        self.lbl_time.pack(anchor="w")

        ttk.Label(status_frame, text="Level Tangki Pakan:", font=("Arial", 11)).pack(anchor="w", pady=(10, 2))
        self.progress_stok = ttk.Progressbar(status_frame, orient="horizontal", length=200, mode="determinate")
        self.progress_stok.pack(anchor="w")
        self.lbl_stok_percent = ttk.Label(status_frame, text="-- %", font=("Arial", 12, "bold"))
        self.lbl_stok_percent.pack(anchor="w")

        # Kanan: Multi-Schedule & DURATION
        jadwal_frame = ttk.LabelFrame(top_frame, text=" Pengaturan Jadwal & Durasi ", padding=(10, 10))
        jadwal_frame.pack(side="right", fill="both", expand=True, padx=(5, 0))

        ttk.Label(jadwal_frame, text="Pagi :").grid(row=0, column=0, sticky="w", pady=2)
        self.ent_pagi = ttk.Entry(jadwal_frame, width=8)
        self.ent_pagi.grid(row=0, column=1, padx=5); self.ent_pagi.insert(0, "07:00")

        ttk.Label(jadwal_frame, text="Siang:").grid(row=1, column=0, sticky="w", pady=2)
        self.ent_siang = ttk.Entry(jadwal_frame, width=8)
        self.ent_siang.grid(row=1, column=1, padx=5); self.ent_siang.insert(0, "12:00")

        ttk.Label(jadwal_frame, text="Sore :").grid(row=2, column=0, sticky="w", pady=2)
        self.ent_sore = ttk.Entry(jadwal_frame, width=8)
        self.ent_sore.grid(row=2, column=1, padx=5); self.ent_sore.insert(0, "16:30")

        btn_set = ttk.Button(jadwal_frame, text="Kirim Jadwal", command=self.send_set_jadwal)
        btn_set.grid(row=3, column=0, columnspan=2, pady=5, sticky="we")

        # --- TAMBAHAN BARU: UI Pengaturan Durasi ---
        ttk.Separator(jadwal_frame, orient="horizontal").grid(row=4, column=0, columnspan=2, sticky="we", pady=5)
        
        ttk.Label(jadwal_frame, text="Durasi Pakan (ms):").grid(row=5, column=0, sticky="w", pady=2)
        self.ent_durasi = ttk.Entry(jadwal_frame, width=8)
        self.ent_durasi.grid(row=5, column=1, padx=5); self.ent_durasi.insert(0, "5000") # Default 5 detik
        
        btn_dur = ttk.Button(jadwal_frame, text="Kirim Durasi", command=self.send_set_durasi)
        btn_dur.grid(row=6, column=0, columnspan=2, pady=5, sticky="we")

        # --- TOMBOL MANUAL FEED ---
        btn_feed = ttk.Button(self.root, text="⚡ EKSEKUSI PAKAN MANUAL", command=self.send_manual_feed)
        btn_feed.pack(fill="x", pady=15, ipady=8)

        # --- PANEL BAWAH: DATA LOGGER / HISTORY ---
        history_frame = ttk.LabelFrame(self.root, text=" History Pemberian Pakan ", padding=(10, 10))
        history_frame.pack(fill="both", expand=True)

        # Scrollbar untuk tabel
        scroll = ttk.Scrollbar(history_frame)
        scroll.pack(side="right", fill="y")

        # Tabel Treeview
        self.tree = ttk.Treeview(history_frame, columns=("Waktu", "Metode", "Status"), show="headings", yscrollcommand=scroll.set)
        self.tree.heading("Waktu", text="Waktu Eksekusi")
        self.tree.heading("Metode", text="Metode")
        self.tree.heading("Status", text="Status Tangki")
        
        self.tree.column("Waktu", width=150, anchor="center")
        self.tree.column("Metode", width=150, anchor="center")
        self.tree.column("Status", width=100, anchor="center")
        
        self.tree.pack(fill="both", expand=True)
        scroll.config(command=self.tree.yview)

    def log_history(self, metode):
        # Ambil waktu PC saat ini untuk log
        waktu_sekarang = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        stok_saat_ini = self.lbl_stok_percent.cget("text")
        # Masukkan ke baris paling atas tabel (index 0)
        self.tree.insert("", 0, values=(waktu_sekarang, metode, stok_saat_ini))

    def connect_socket(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(1) 
            self.sock.connect((HOST, PORT))
            self.lbl_time.config(text="Status: Terhubung ke Wokwi")
            
            self.read_thread = threading.Thread(target=self.read_from_mcu)
            self.read_thread.daemon = True
            self.read_thread.start()
        except Exception as e:
            self.lbl_time.config(text=f"ERROR: Wokwi belum 'Play'")

    def read_from_mcu(self):
        buffer = ""
        while self.is_running and self.sock:
            try:
                data = self.sock.recv(1024).decode('utf-8')
                if data:
                    buffer += data
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        self.parse_data(line.strip())
            except socket.timeout:
                pass
            except Exception as e:
                pass
            time.sleep(0.05)

    def parse_data(self, data):
        try:
            # PUSAT LOG DATA LOGGER (Menerima perintah mutlak dari MCU)
            if data.startswith("EVENT:"):
                event_type = data.split("EVENT:")[1]
                if event_type == "AUTO":
                    self.log_history("Otomatis (Jadwal)")
                elif event_type == "PHYSICAL":
                    self.log_history("Manual (Tombol Fisik)")
                elif event_type == "GUI":
                    self.log_history("Manual (GUI Python)")
                return # Langsung keluar fungsi setelah logging

            # Pemisahan status standar
            parts = data.split('|')
            for part in parts:
                if part.startswith("TIME:"):
                    waktu = part.split("TIME:")[1]
                    self.lbl_time.config(text=f"Jam MCU : {waktu}")
                elif part.startswith("STOK:"):
                    stok = int(part.split("STOK:")[1])
                    self.progress_stok['value'] = stok
                    self.lbl_stok_percent.config(text=f"{stok} %")
        except Exception as e:
            pass

    def send_manual_feed(self):
        if self.sock:
            try:
                self.sock.sendall(b'#FEED\n')
                print("Command terkirim via TCP: #FEED")
            except Exception as e:
                print("Gagal mengirim perintah:", e)

    def send_set_jadwal(self):
        if self.sock:
            # Ambil ke-3 inputan
            p = self.ent_pagi.get().strip()
            si = self.ent_siang.get().strip()
            so = self.ent_sore.get().strip()
            
            # Format pengiriman: #SET:07:00,12:00,16:30
            if len(p)==5 and len(si)==5 and len(so)==5:
                command = f"#SET:{p},{si},{so}\n"
                try:
                    self.sock.sendall(command.encode('utf-8'))
                    print(f"Jadwal baru dikirim: {p}, {si}, {so}")
                except Exception as e:
                    print("Gagal mengirim jadwal")
            else:
                print("Format salah! Pastikan ketiganya berformat HH:MM")

    # --- TAMBAHAN BARU: Fungsi Mengirim Durasi ---
    def send_set_durasi(self):
        if self.sock:
            # Ambil inputan angka
            dur_str = self.ent_durasi.get().strip()
            if dur_str.isdigit(): # Pastikan isinya hanya angka
                command = f"#DUR:{dur_str}\n"
                try:
                    self.sock.sendall(command.encode('utf-8'))
                    print(f"Durasi pakan baru dikirim: {dur_str} ms")
                except Exception as e:
                    print("Gagal mengirim durasi")
            else:
                print("Format salah! Masukkan angka dalam milidetik (misal: 3000)")

    def on_closing(self):
        self.is_running = False
        if self.sock:
            self.sock.close()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = ManganPitikHMI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()