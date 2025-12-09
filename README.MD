ESP32 Attendance System – Attack & Defense Lab
Repositori ini digunakan untuk demonstrasi pengujian keamanan dasar (Attack Simulation) dan perbaikan keamanan (Defense) pada sistem absensi ESP32 yang mengirim data ke Google Spreadsheet menggunakan Google Apps Script.

⚠️ Catatan: Semua langkah di sini hanya untuk lab uji coba pendidikan.
Penyalahgunaan di luar itu adalah tanggung jawab Anda sepenuhnya.

## 1. Cara Melakukan Attack Simulation (DDoS ke Endpoint Google Apps Script)
Tujuan bagian ini: memahami bagaimana endpoint yang diekspos (URL Apps Script) dapat dieksploitasi ketika tampil di Serial Monitor ESP32.

A. Hubungkan ESP32 ke Laptop
Gunakan kabel USB → pastikan ESP32 terdeteksi.

B. Dapatkan URL Google Apps Script
Atur VirtualBox USB (jika pakai VM).
Pastikan ESP32 terdeteksi oleh VirtualBox.

Masuk ke Linux, buka terminal.

Cek device ESP32:

ls /dev/tty*
Buka serial monitor menggunakan picocom:

picocom -b 115200 /dev/ttyUSB0
Lihat output dan ambil URL Google Apps Script yang muncul dari ESP32.

C. Lakukan Attack (Simulasi DDoS)
Gunakan PyCharm untuk membuat script Python sederhana yang mengirim request terus-menerus ke URL tersebut.

Masukkan URL yang sudah kamu dapatkan dari serial tadi.

Script ini akan membanjiri endpoint sehingga:

Google Sheet menerima terlalu banyak request

Spreadsheet penuh

Apps Script mulai menolak request

Tujuan lab: memahami titik lemah sistem absensi yang menggunakan public endpoint.

D. Verifikasi Serangan
Periksa Google Spreadsheet:

Jika baris bertambah sangat cepat → serangan berhasil

Jika Apps Script mulai mengembalikan error → sistem overload

## 2. Cara Defense (Mengamankan Sistem)
Masalah utama:
URL Google Apps Script terekspos di Serial Monitor, sehingga bisa dicuri siapa saja.

A. Perbaikan pada Google Apps Script
Tambahkan Secret Code di parameter, contoh:

function doGet(e) {
  if (e.parameter.secret !== "KODE_RAHASIA_KAMU") {
    return ContentService.createTextOutput("UNAUTHORIZED");
  }
  // proses simpan data
}
B. Perbaikan pada ESP32
Tambahkan pengiriman secret code di request HTTP.

Hapus URL dari Serial Monitor agar tidak bisa diambil penyerang.

ESP32 hanya mengirim:

http.begin("https://script.google.com/.../exec?secret=KODE_RAHASIA_KAMU");
Tidak ada lagi log URL di Serial.

## Inti dari Semua Ini
Attack: Menunjukkan betapa rentannya sistem jika URL API terpapar.

Defense: Menutup celah dengan secret key + tidak menampilkan endpoint di Serial.
