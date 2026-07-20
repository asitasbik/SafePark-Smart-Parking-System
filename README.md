# SafePark - Akıllı Otopark Yönetim Sistemi

SafePark, otopark alanlarında araç varlık tespiti ve kapasite takibini otomatikleştirmek amacıyla geliştirilmiş gömülü sistem tabanlı bir IoT prototipidir. Sistem, anlık veri işleme mekanizmasıyla sürücüleri ve yöneticileri bilgilendirir.

## Mimari ve Kullanılan Teknolojiler

* **Mikrodenetleyici:** ESP32 (Dual-Core 32-bit MCU)
* **Sensör Mimarisi:** HC-SR04 Ultrasonik Mesafe Sensörleri
* **Görüntüleme Birimi:** SSD1306 OLED Ekran (I2C Haberleşme Protokolü)
* **Programlama Dili & Ortam:** C / C++ (Arduino Framework / PlatformIO)

## Çalışma Prensibi

1. **Veri Toplama:** Park alanlarına konumlandırılan ultrasonik sensörler yardımıyla belirli mesafe eşikleri üzerinden araç tespiti yapılır.
2. **Yerel Durum Gösterimi:** Her park yerindeki dedektörler, alanın "DOLU" veya "BOŞ" durumunu bağlı bulunan lokal ekranlara iletir.
3. **Merkezi Kapasite Yönetimi:** Giriş biriminde yer alan ana ekran, tüm sensörlerden gelen verileri işleyerek toplam boş kapasiteyi gerçek zamanlı olarak günceller.

## Kurulum ve Dağıtım

1. Proje deposunu yerel ortama klonlayın.
2. `Adafruit_SSD1306` ve `Adafruit_GFX` kütüphanelerini geliştirme ortamınıza dahil edin.
3. `SafePark.ino` içerisindeki pin konfigürasyonlarına uygun olarak donanım bağlantılarını gerçekleştirin.
4. Derlenen kod mimarisini ESP32 kartına yükleyin.
