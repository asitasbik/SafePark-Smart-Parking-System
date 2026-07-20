#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- Wi-Fi ve Telegram Ayarları ---
const char* ssid = "Efe_WiFi";         // iPhone 13 hotspot adın
const char* password = "12345678";     // Hotspot şifren
#define BOTtoken "BOT_TOKENIN_BURAYA"  // BotFather'dan aldığın token
#define CHAT_ID "CHAT_ID_BURAYA"       // IDBot'tan aldığın chat ID

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long sonTelegramKontrol = 0;
const int telegramGecikmesi = 2000;

// --- EKRAN AYARI (AYNI PİNDE İKİZ EKRAN) ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 4
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

const int trig1 = 5, echo1 = 18;
const int trig2 = 19, echo2 = 23;
const int RED_LED = 2;
const int GREEN_LED = 14;

float canliIsi = 0.0;
bool yanginMesajiAtildi = false;
bool wifiBagli = false; // Offline koruması

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  pinMode(trig1, OUTPUT); pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT); pinMode(echo2, INPUT);
  pinMode(RED_LED, OUTPUT); pinMode(GREEN_LED, OUTPUT);

  // Ekranı Başlat
  Wire.begin(21, 22); 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("WiFi Bekleniyor...");
  display.display();

  // --- Wi-Fi'ye Bağlanma (Zaman Aşımı Korumalı) ---
  WiFi.begin(ssid, password);
  int deneme = 0;
  while (WiFi.status() != WL_CONNECTED && deneme < 15) {
    delay(500);
    deneme++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiBagli = true;
    client.setInsecure();
    display.clearDisplay(); display.setCursor(0, 20); display.print("WiFi BAGLANDI!"); display.display();
  } else {
    wifiBagli = false;
    display.clearDisplay(); display.setCursor(0, 20); display.print("WiFi YOK! Offline"); display.display();
  }
  delay(1500);
}

long mesafe(int t, int e) {
  digitalWrite(t, LOW); delayMicroseconds(2);
  digitalWrite(t, HIGH); delayMicroseconds(10);
  digitalWrite(t, LOW);
  return pulseIn(e, HIGH, 20000) * 0.034 / 2;
}

// Bota Gelen Soruları İşleme
void yeniMesajlariIsle(int mesajSayisi, bool p1, bool p2, float isi, bool fire) {
  for (int i = 0; i < mesajSayisi; i++) {
    String gelenChatId = bot.messages[i].chat_id;
    String gelenMetin = bot.messages[i].text;
    
    if (gelenMetin == "?") {
      String cevap = "🚗 SAFEPARK CANLI DURUM RAPORU 🚗\n\n";
      cevap += "🅿️ P1 Alanı: " + String(p1 ? "🔴 DOLU" : "🟢 BOS") + "\n";
      cevap += "🅿️ P2 Alanı: " + String(p2 ? "🔴 DOLU" : "🟢 BOS") + "\n";
      cevap += "🌡️ İç Sıcaklık: " + String(isi, 1) + " C\n\n";
      
      if (fire) cevap += "⚠️ DİKKAT: Otoparkta Yangın Alarmı Var!";
      else if (p1 && p2) cevap += "⛔ Otoparkımız şu an tam kapasite doludur.";
      else cevap += "✅ Otoparkımızda boş yer mevcuttur.";
      
      bot.sendMessage(gelenChatId, cevap, "");
    }
  }
}

void loop() {
  float okunan = dht.readTemperature();
  if (!isnan(okunan) && okunan > -10.0) canliIsi = okunan;

  long d1 = mesafe(trig1, echo1);
  long d2 = mesafe(trig2, echo2);
  bool p1_dolu = (d1 > 0 && d1 < 10);
  bool p2_dolu = (d2 > 0 && d2 < 10);
  bool fire = (canliIsi > 26.0); 

  // --- TELEGRAM KONTROLLERİ ---
  if (wifiBagli) {
    if (fire && !yanginMesajiAtildi) {
      String mesaj = "⚠️ ACİL DURUM: Otoparkta Yangın Riski! \nSıcaklık: " + String(canliIsi) + " C\nGirişler Kapatıldı.";
      bot.sendMessage(CHAT_ID, mesaj, "");
      yanginMesajiAtildi = true; 
    } 
    else if (!fire) {
      yanginMesajiAtildi = false; 
    }

    if (millis() - sonTelegramKontrol > telegramGecikmesi) {
      int yeniMesajlar = bot.getUpdates(bot.last_message_received + 1);
      while (yeniMesajlar) {
        yeniMesajlariIsle(yeniMesajlar, p1_dolu, p2_dolu, canliIsi, fire);
        yeniMesajlar = bot.getUpdates(bot.last_message_received + 1);
      }
      sonTelegramKontrol = millis();
    }
  }

  // --- LED KONTROLÜ ---
  if (fire) {
    digitalWrite(RED_LED, HIGH); delay(100); digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
  } else {
    digitalWrite(GREEN_LED, (p1_dolu && p2_dolu) ? LOW : HIGH);
    digitalWrite(RED_LED, (p1_dolu && p2_dolu) ? HIGH : LOW);
  }

  // --- EKRAN GÖSTERİMİ ---
  display.clearDisplay();
  display.setTextColor(WHITE);
  if (fire) {
    display.setTextSize(2); display.setCursor(20, 25); display.print("YANGIN!");
  } else {
    display.setTextSize(2); display.setCursor(0, 0);
    display.print("P1: "); display.println(p1_dolu ? "DOLU" : "BOS");
    display.print("P2: "); display.println(p2_dolu ? "DOLU" : "BOS");
    display.setCursor(0, 48); display.print("ISI:"); display.print(canliIsi, 1); display.print("C");
  }
  display.display();
  
  delay(500); 
}