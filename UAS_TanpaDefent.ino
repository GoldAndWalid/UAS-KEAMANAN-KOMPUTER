//----------------------------------------Including the libraries.
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
//----------------------------------------

// Defines SS/SDA PIN and Reset PIN for RFID-RC522.
#define SS_PIN  5    // G5
#define RST_PIN 4    // G4

// Defines the button PIN.
#define BTN_PIN 15
#define BUZZER_PIN 12

//----------------------------------------
const char* ssid = "Hospot";  //--> nama wifi kamu
const char* password = "1234567890"; //--> password wifi
//----------------------------------------

// Google script Web_App_URL (PASTIKAN INI URL DEPLOY TERBARU ANDA)
String Web_App_URL = "https://script.google.com/macros/s/AKfycbwmkxdgNAx-zlyXrOEg5KFeZPFrcrx8pScQEzyP3CWl3tencolP6qxl-_MEATSYLTug/exec";

String reg_Info = "";

String atc_Role = "";
String atc_Info = "";
String atc_Name = "";
String atc_Date = "";
String atc_Time_In = "";
String atc_Time_Out = "";
String current_role = "";

// Variables for the number of columns and rows on the LCD.
int lcdColumns = 16;
int lcdRows = 2;

// Variable to read data from RFID-RC522.
int readsuccess;
char str[32] = "";

String UID_Result = "--------";

String modes = "atc";
bool show_fast_success = false; 

bool admin_access_granted = false;  // Untuk mengontrol akses mode REG oleh admin

// Initialize LCD
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// Create MFRC522 instance.
MFRC522 mfrc522(SS_PIN, RST_PIN);  

//__getValue()
// String function to process the data (Split String).
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//__http_Req()
void http_Req(String str_modes, String str_uid, String mode_type) {
  if (WiFi.status() == WL_CONNECTED) {
    String http_req_url = "";

    //----------------------------------------Create links to make HTTP requests
    if (str_modes == "atc") {
      http_req_url  = Web_App_URL + "?sts=atc";
      http_req_url += "&uid=" + str_uid;
      http_req_url += "&mode_type=" + mode_type;
    }

    if (str_modes == "reg") {
      http_req_url = Web_App_URL + "?sts=reg";
      http_req_url += "&uid=" + str_uid;
    }
    //----------------------------------------

    //----------------------------------------Sending HTTP requests
    Serial.println();
    Serial.println("-------------");
    Serial.println("Sending request to Google Sheets...");
    Serial.print("URL : ");
    Serial.println(http_req_url);
    
    HTTPClient http;

    // HTTP GET Request.
    http.begin(http_req_url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // Gets the HTTP status code.
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);

    // Tambahan untuk mendeteksi jika gagal kirim data
    if (httpCode <= 0) {
      Serial.println("GAGAL MENGIRIMKAN DATA KE SPREADSHEET!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error Kirim Data!");
      lcd.setCursor(0, 1);
      lcd.print("InternetBermasalah");
      delay(4000);
      lcd.clear();
      http.end();
      return; 
    }

    // Getting response from google sheet.
    String payload;
    if (httpCode > 0) {
      payload = http.getString();
      Serial.println("Payload : " + payload);

      // =================================================================
      // --- MODIFIKASI: CEK KHUSUS BATAS PENDAFTARAN (QUOTA USER) ---
      // =================================================================
      if (payload.indexOf("FULL_REG") != -1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("QUOTA PENUH !");
        lcd.setCursor(0, 1);
        lcd.print("CAN'T REGISTER");
        
        // Bunyi Beep Panjang
        digitalWrite(BUZZER_PIN, HIGH);
        delay(2000); 
        digitalWrite(BUZZER_PIN, LOW);
        
        delay(2000); 
        lcd.clear();
        http.end();
        return; // Stop proses disini
      }
      // =================================================================

      if (mode_type == "fast" && payload.indexOf("TO_") != -1) {
        show_fast_success = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SELAMAT PULANG");
        lcd.setCursor(0, 1);
        lcd.print("HATI-HATI YA :)");
        delay(4000);
        lcd.clear();
        show_fast_success = false;
      }
    }
    
    Serial.println("-------------");
    http.end();
    
    String sts_Res = getValue(payload, ',', 0);

    //----------------------------------------Logic Response Payload
    if (sts_Res == "OK") {
      if (str_modes == "atc") {
        atc_Info = getValue(payload, ',', 1);
        atc_Role = getValue(payload, ',', 5); 

        if (atc_Info == "TI_Successful") {
          atc_Name = getValue(payload, ',', 2);
          atc_Date = getValue(payload, ',', 3);
          atc_Time_In = getValue(payload, ',', 4);
          String atc_Role = getValue(payload, ',', 5); 

          if (atc_Role == "admin" || atc_Role == "admin") {
            admin_access_granted = true;
            Serial.println("Akses admin diberikan setelah Time In");
          }

          int name_Lenght = atc_Name.length();
          int pos = 0;
          if (name_Lenght > 0 && name_Lenght <= lcdColumns) {
            pos = map(name_Lenght, 1, lcdColumns, 0, (lcdColumns / 2) - 1);
            pos = ((lcdColumns / 2) - 1) - pos;
          } else if (name_Lenght > lcdColumns) {
            atc_Name = atc_Name.substring(0, lcdColumns);
          }

          lcd.clear();
          delay(500);
          lcd.setCursor(pos,0);
          lcd.print(atc_Name);
        
          lcd.setCursor(0,1);
          lcd.print("DATANG :");
          lcd.print(atc_Time_In);
          delay(5000);
          lcd.clear();
          delay(500);
        }

        if (atc_Info == "TO_Successful") {
          atc_Name = getValue(payload, ',', 2);
          atc_Date = getValue(payload, ',', 3);
          atc_Time_In = getValue(payload, ',', 4);
          atc_Time_Out = getValue(payload, ',', 5);
          String role = getValue(payload, ',', 6);
          
          if (role.equalsIgnoreCase("admin")) {
            admin_access_granted = true;
            Serial.println("Akses admin diberikan setelah Time Out");
          }

          int name_Lenght = atc_Name.length();
          int pos = 0;
          if (name_Lenght > 0 && name_Lenght <= lcdColumns) {
            pos = map(name_Lenght, 1, lcdColumns, 0, (lcdColumns / 2) - 1);
            pos = ((lcdColumns / 2) - 1) - pos;
          } else if (name_Lenght > lcdColumns) {
            atc_Name = atc_Name.substring(0, lcdColumns);
          }

          lcd.clear();
          delay(500);
          lcd.setCursor(pos,0);
          lcd.print(atc_Name);
          
          lcd.setCursor(0,1);
          lcd.print("PULANG:");
          lcd.print(atc_Time_Out);
          delay(5000);
          lcd.clear();
          delay(500);
        }

        if (atc_Info == "atcInf01") {
          atc_Name = getValue(payload, ',', 2);    
          current_role = getValue(payload, ',', 3);  
          Serial.println("Payload Info: " + atc_Info);
          Serial.println("Role Result: " + current_role);

          if (current_role == "admin") {
            admin_access_granted = true;
            Serial.println("Akses ADMIN diaktifkan");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("ADMIN MODE AKTIF");
            delay(2000);
            admin_access_granted = true;
          } else {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("KAMU SUDAH");
            lcd.setCursor(0,1);
            lcd.print("PRESENSI HARI INI");
          }

          delay(5000);
          lcd.clear();
          delay(500);
        }

        else if (atc_Info == "TO_Early") {
          atc_Name = getValue(payload, ',', 2);
          atc_Role = getValue(payload, ',', 3);

          if (atc_Role.equalsIgnoreCase("admin")) {
            admin_access_granted = true;  
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("AKSES ADMIN");
            lcd.setCursor(0, 1);
            lcd.print("MODE REG OK");
            delay(3000);

          } else {
            lcd.clear();
            delay(500);
            lcd.setCursor(0, 0);
            lcd.print("BELUM WAKTUNYA");
            lcd.setCursor(0, 1);
            lcd.print("PULANG BRO!");
            delay(200);

            digitalWrite(BUZZER_PIN, HIGH);
            delay(4000);
            digitalWrite(BUZZER_PIN, LOW);

            lcd.clear();
            delay(500);
          }
        }
        if (atc_Info == "atcErr01") {
          lcd.clear();
          delay(500);

          lcd.setCursor(0,0);
          lcd.print("Error !");
          lcd.setCursor(0,1);
          lcd.print("TIDAK TERDAFTAR");
          delay(200);
          digitalWrite(BUZZER_PIN, HIGH);
          delay(5000);
          digitalWrite(BUZZER_PIN, LOW);
          lcd.clear();
          delay(500);
        }

        atc_Info = "";
        atc_Name = "";
        atc_Date = "";
        atc_Time_In = "";
        atc_Time_Out = "";
      }
      
      if (str_modes == "reg") {
        reg_Info = getValue(payload, ',', 1);
        
        if (reg_Info == "regErr01") {
          lcd.clear();
          delay(500);
          lcd.setCursor(0,0);
          lcd.print("ERROR !");
          lcd.setCursor(0,1);
          lcd.print("KARTU SUDAH ADA");
          delay(5000);
          lcd.clear();
          delay(500);
        }
        else if (reg_Info == "R_Successful") {
          lcd.clear();
          delay(500);
          lcd.setCursor(0,0);
          lcd.print("DAFTAR BERHASIL");
          lcd.setCursor(0,1);
          lcd.print("UID : ");
          lcd.print(UID_Result);
          delay(5000);
          lcd.clear();
          delay(500);
        }

        reg_Info = "";
      }
    }
  } else {
    lcd.clear();
    delay(500);
    lcd.setCursor(6,0);
    lcd.print("Error !");
    lcd.setCursor(1,1);
    lcd.print("WiFi disconnected");
    delay(3000);
    lcd.clear();
    delay(500);
  }
}

//__getUID()
int getUID() {  
  if(!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if(!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  
  byteArray_to_string(mfrc522.uid.uidByte, mfrc522.uid.size, str);
  UID_Result = str;
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  return 1;
}

//__byteArray_to_string()
void byteArray_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len*2] = '\0';
}

//________________________________________________________________________________
// FUNGSI BARU UNTUK CEK WIFI YANG LEBIH STABIL
void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi terputus. Mencoba menghubungkan ulang...");
    
    // Tampilkan di LCD sebentar
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sinyal Hilang");
    lcd.setCursor(0, 1);
    lcd.print("Reconnecting...");
    
    // Coba reconnect dengan metode standar
    WiFi.disconnect();
    WiFi.reconnect();
    
    // Tunggu sebentar (non-blocking jika bisa, tapi untuk keamanan kita beri delay dikit)
    int retry = 0;
    while(WiFi.status() != WL_CONNECTED && retry < 10) {
      delay(500);
      Serial.print(".");
      retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Konek Lagi");
      delay(1500);
      lcd.clear();
    }
  }
}

//__VOID SETUP()
void setup(){
  Serial.begin(115200);
  Serial.println();
  delay(1000);

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 

  // Initialize LCD.
  lcd.init();
  lcd.backlight();
  lcd.clear();
  delay(500);

  // Init SPI bus & MFRC522.
  SPI.begin();      
  mfrc522.PCD_Init(); 

  delay(500);

  lcd.setCursor(0,0);
  lcd.print("WELCOME TO");
  lcd.setCursor(0,1);
  lcd.print("PRESENSI ONLINE");
  delay(3000);
  lcd.clear();

  //----------------------------------------WIFI CONNECTION (PERBAIKAN UTAMA DISINI)
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true); // Biarkan ESP32 otomatis handle reconnect
  
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // LOOP YANG LEBIH SABAR (Tunggu sampai 40 detik / 80 kali loop)
  // TIDAK ADA ESP.RESTART DISINI AGAR TIDAK BOOTLOOP
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 80) {
    delay(500);
    Serial.print(".");
    
    // Animasi simple di LCD
    lcd.setCursor(0,0);
    lcd.print("Menghubungkan...");
    lcd.setCursor(0,1);
    if(attempt % 3 == 0) lcd.print(".          ");
    else if(attempt % 3 == 1) lcd.print("..         ");
    else lcd.print("...        ");
    
    attempt++;
  }

  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi TERSAMBUNG");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WiFi TERSAMBUNG");
    lcd.setCursor(0,1);
    lcd.print("Siap Digunakan");
  } else {
    Serial.println("Gagal Terhubung (Timeout).");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Gagal Konek WiFi");
    lcd.setCursor(0,1);
    lcd.print("Cek Hotspot HP!");
    // Kita biarkan lanjut ke loop(), siapa tau nanti nyambung sendiri
  }
  
  delay(2000);
  lcd.clear();
  delay(500);
}

//__VOID LOOP()
void loop(){
  // Panggil fungsi cek wifi (tapi tidak se-agresif kode sebelumnya)
  checkWiFiConnection();
  
  if (admin_access_granted) {
  }

  //----------------------------------------Switches modes
  int BTN_State = digitalRead(BTN_PIN);

  if (BTN_State == LOW) {
    lcd.clear();
     
    // Logika tombol
    if (modes == "atc") {
      if (admin_access_granted) {
        modes = "reg";
      } else {
        modes = "fast"; 
      }
    } 
    else if (modes == "reg") {
      modes = "atc";
      admin_access_granted = false;  
    } else if (modes == "fast") {
      modes = "atc";
    }
    
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);
  }
  //----------------------------------------

  // Detect RFID
  readsuccess = getUID();

  //----------------------------------------Conditions if modes == "atc".
  if (modes == "atc") {
    lcd.setCursor(0, 0);
    lcd.print("SELAMAT DATANG");
    lcd.setCursor(0, 1);
    lcd.print("TAP KARTU ANDA");

    if (readsuccess) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);

      lcd.clear();
      delay(500);
      lcd.setCursor(0, 0);
      lcd.print("TUNGGU SEBENTAR");
      lcd.setCursor(0, 1);
      lcd.print("MENGIRIMKAN DATA");
      delay(1000);

      http_Req("atc", UID_Result, "normal");
    }
  }
  else if (modes == "fast") {
      lcd.setCursor(0, 0);
      lcd.print("PULANG CEPAT");
      lcd.setCursor(0, 1);
      lcd.print("TAP KARTU ANDA");
    
    if (readsuccess) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);

      lcd.clear();
      delay(500);
      lcd.setCursor(0, 0);
      lcd.print("TUNGGU SEBENTAR");
      lcd.setCursor(0, 1);
      lcd.print("MENGIRIMKAN DATA");
      delay(1000);

      http_Req("atc", UID_Result, "fast");
    }
  }

  //----------------------------------------Conditions if modes == "reg".
  if (modes == "reg") {
    lcd.setCursor(0,0);
    lcd.print("MODE PENDAFTARAN");
    lcd.setCursor(0,1);
    lcd.print("TAP KARTU ANDA");
    
    if (readsuccess){
      digitalWrite(BUZZER_PIN, HIGH);
      delay(150);
      digitalWrite(BUZZER_PIN, LOW);

      lcd.clear();
      delay(500);
      
      lcd.setCursor(0,0);
      lcd.print("TUNGGU SEBENTAR");
      lcd.setCursor(0,1);
      lcd.print("SEDANG CEK DATA");
      delay(1000);

      http_Req(modes, UID_Result, "");
    }
  }

delay(10);
}