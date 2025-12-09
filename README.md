# UAS-KEAMANAN-KOMPUTER

# ANGGOTA KELOMPOK 3
- Harwidi Wahyu Eka Saputra (09030282327036)
- Akbar Arnoval S (09030282327066)
- M Walid Farhan (09030282327039)
- Julian Afriansyah (09030282327024)
- Rangga Saputra (09030282327051)

# Sistem Absensi Database Google Spreadsheet  
Berbasis *RFID, ESP32, LCD I2C, Buzzer & Push Button*

Proyek ini merupakan sistem absensi otomatis menggunakan kartu RFID yang terintegrasi dengan *Google Spreadsheet* sebagai database. Sistem ini menampilkan status absensi melalui *LCD I2C, memberikan respon suara melalui **buzzer, serta menyediakan tombol **push button* untuk fitur tambahan seperti pulang cepat.

---

## 🚀 Fitur Utama
- *Absensi Masuk*  
  Pengguna melakukan tap kartu RFID, data otomatis dikirim ke Google Spreadsheet.

- *Pendaftaran Kartu RFID*  
  Mode pendaftaran untuk menambah ID baru ke sistem.

- *Pulang Cepat*  
  Tombol push button digunakan untuk melakukan input pulang cepat.

- *Notifikasi LCD I2C*  
  Menampilkan status: “Scan Kartu”, “Absensi Berhasil”, “ID Tidak Dikenal”, dll.

- *Buzzer*  
  Memberikan konfirmasi bunyi untuk keberhasilan/penolakan absensi.

---

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

---

## 📡 Alur Kerja Sistem
1. ESP32 terhubung ke Wi-Fi.  
2. Pengguna men-tap kartu RFID.  
3. ESP32 mengirim ID kartu ke Google Apps Script via HTTP Request.  
4. Apps Script menulis data ke Google Spreadsheet.  
5. LCD I2C menampilkan status absensi.  
6. Buzzer memberikan notifikasi.  
7. Jika push button ditekan → mode Pulang Cepat.

---


## 🔧 Cara Instalasi & Setup

### 1. Siapkan Google Spreadsheet
1. Buat spreadsheet baru  
2. Buat header: ID | Nama | Waktu | Status

### 2. Buat Google Apps Script
1. Buka *Extensions → Apps Script*  
2. Tempelkan kode HTTP endpoint untuk menerima data dari ESP32  
3. Deploy sebagai *Web App* (akses: anyone / anyone with link)  
4. Salin URL API Web App

### 3. Upload Kode ke ESP32
1. Bukalah Arduino IDE / PlatformIO  
2. Install library:
   - MFRC522
   - LiquidCrystal_I2C
   - WiFi.h  
3. Masukkan:
   - SSID & password Wi-Fi  
   - URL Apps Script  

4. Upload kode ke ESP32

---

## ▶ Cara Menggunakan
1. Nyalakan perangkat absensi.  
2. LCD menampilkan *“Scan Kartu”*.  
3. Tap kartu RFID:  
   - Jika dikenal → dicatat sebagai *Masuk*  
   - Jika tombol ditekan → *Pulang Cepat*  
4. Data langsung masuk ke Google Spreadsheet.  

---

## 📜 Lisensi
Proyek ini bebas digunakan untuk kebutuhan pembelajaran, magang, dan pengembangan internal.

---

## 👨‍💻 Pengembang
Akbar Arnoval S  
Dinas Perindustrian Provinsi Sumatera Selatan
