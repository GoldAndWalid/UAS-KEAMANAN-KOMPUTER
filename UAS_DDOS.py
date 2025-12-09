import requests
import threading
import time
import uuid
import random

# ================= KONFIGURASI =================
# Pastikan URL ini benar
url_target = "Masukan"

# BATAS PENGIRIMAN
TARGET_LIMIT = 300
# ===============================================

total_requests = 0
lock = threading.Lock()  # Kunci agar hitungan tidak bentrok antar thread


def attack_google():
    global total_requests
    while True:
        # --- BAGIAN PENGECEKAN JUMLAH ---
        with lock:
            if total_requests >= TARGET_LIMIT:
                break  # Berhenti jika sudah 130

            total_requests += 1
            current_request_num = total_requests
        # --------------------------------

        try:
            # 1. BUAT UID UNIK
            unique_uid = str(uuid.uuid4())[:8].upper()

            # 2. BUAT NAMA PALSU
            random_name = f"BOT_USER_{random.randint(100, 9999)}"

            # 3. SETTING URL
            full_url = f"{url_target}?sts=reg&uid={unique_uid}&nama={random_name}"

            # Kirim request
            response = requests.get(full_url, timeout=5)

            if response.status_code == 200 or response.status_code == 302:
                print(f"[{current_request_num}/{TARGET_LIMIT}] SUKSES: {unique_uid}")
            elif response.status_code == 429:
                print(f"[{current_request_num}/{TARGET_LIMIT}] LIMIT (Server Sibuk)")
            else:
                print(f"[{current_request_num}/{TARGET_LIMIT}] Status: {response.status_code}")

            # Jeda sedikit
            time.sleep(0.5)

        except Exception as e:
            # Jika error koneksi, kurangi hitungan agar dicoba lagi (opsional)
            # Tapi untuk aman, kita biarkan jalan terus
            print(f"[{current_request_num}] Error Koneksi")
            time.sleep(1)


print(f"MEMULAI PENGIRIMAN {TARGET_LIMIT} DATA...")

jumlah_threads = 20
threads = []

# Membuat Thread
for i in range(jumlah_threads):
    t = threading.Thread(target=attack_google)
    t.daemon = True  # Agar thread mati saat program utama selesai
    threads.append(t)

# Menjalankan Thread
for t in threads:
    t.start()

# Menunggu sampai semua thread selesai bekerja (mencapai limit)
while True:
    if total_requests >= TARGET_LIMIT:
        print("\n========================================")
        print(f"TARGET TERCAPAI: {total_requests} PERMINTAAN DIKIRIM.")
        print("PROGRAM BERHENTI.")
        print("========================================")
        break
time.sleep(1)
