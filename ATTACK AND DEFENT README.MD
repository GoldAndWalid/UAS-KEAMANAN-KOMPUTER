

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

# 🎯 Tujuan Lab
1.  **Reconnaissance:** Memahami bagaimana informasi sensitif (URL Endpoint) bisa bocor melalui Serial Monitor.
2.  **Exploitation:** Melakukan simulasi serangan (*flooding*) ke endpoint yang terekspos.
3.  **Hardening:** Mengamankan sistem dengan mekanisme *Secret Token* dan sanitasi log.

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

**Would you like me to create the `attack.py` script file content for you to include in the repo as well?**
```
