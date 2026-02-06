# UAS Sistem Mikroprosessor : Nur Anisa 23552011171

# ESP32 IoT Control Dashboard

Sistem embedded berbasis **ESP32** yang mengimplementasikan kontrol LED PWM, konfigurasi WiFi dinamis, dashboard web real-time, dan komunikasi MQTT. Proyek ini dirancang untuk memenuhi konsep manajemen mikroprosesor: interupsi, multitasking, manajemen memori, dan komunikasi jaringan.

---

## 📌 Fitur Utama

- Interrupt tombol tanpa polling
- Kontrol LED menggunakan PWM (software PWM)
- Dashboard web berbasis HTTP
- Penyimpanan konfigurasi WiFi permanen (Preferences)
- Komunikasi MQTT real-time
- Multitasking menggunakan FreeRTOS
- Mode Access Point otomatis jika WiFi gagal

---

## 🧠 Konsep Mikroprosesor yang Digunakan

### 1. Interrupt Handling

Input tombol menggunakan hardware interrupt agar CPU tidak melakukan polling terus-menerus.

```cpp
attachInterrupt(BUTTON_PIN, handleButton, FALLING);
```

Interrupt membuat sistem responsif tanpa membebani prosesor.

---

### 2. Multitasking (FreeRTOS)

ESP32 menjalankan task MQTT secara paralel dengan web server dan PWM.

```cpp
xTaskCreatePinnedToCore(mqttTask, "mqtt", 4096, NULL, 1, NULL, 1);
```

Manfaat:
- Proses jaringan tidak mengganggu kontrol hardware
- Sistem tetap real-time

---

### 3. Manajemen Memori

Konfigurasi WiFi disimpan di memori non-volatile menggunakan Preferences.

```cpp
prefs.putString("ssid", ssid);
prefs.putString("password", password);
```

Data tetap tersimpan meskipun perangkat dimatikan.

---

### 4. Komunikasi Data

Sistem menggunakan dua metode komunikasi:

- HTTP Web Server → Dashboard lokal
- MQTT → komunikasi IoT

Contoh payload MQTT:

```json
{
  "led": true,
  "brightness": 120
}
```

---

## 🌐 Arsitektur Sistem

```
Tombol (Interrupt)
       |
       v
    ESP32
 ┌───────────────┐
 │ FreeRTOS Task │
 │ PWM Control   │
 │ Web Server    │
 │ MQTT Client   │
 └───────┬───────┘
         │
   WiFi Network
         │
 MQTT Broker + Web Browser
```

---

## 🚀 Cara Menjalankan

### 1. Upload Program

Gunakan Arduino IDE / PlatformIO

Board:

```
ESP32 Dev Module
```

---

### 2. Boot Pertama

ESP32 masuk mode Access Point:

```
SSID: ESP32-Config
IP  : 192.168.4.1
```

Hubungkan perangkat ke WiFi tersebut lalu buka:

```
http://192.168.4.1
```

Masukkan SSID dan password WiFi rumah.

---

### 3. Akses Dashboard

Setelah terkoneksi WiFi:

```
http://esp32.local
```

atau cek IP di Serial Monitor.

---

## 📡 MQTT Configuration

Broker default:

```
broker.hivemq.com
Topic: esp32/dashboard
```

Monitoring dapat dilakukan dengan:

- MQTT Explorer
- HiveMQ Web Client
- Node-RED

---

## 🧩 Struktur Program

```
setup()
 ├── init WiFi
 ├── init Web Server
 ├── init MQTT
 └── create FreeRTOS task

loop()
 ├── handle web client
 ├── software PWM
 └── interrupt logic
```

---

## ⚙️ Hardware

- ESP32
- LED + resistor
- Push button
- USB power

---

## ✅ Output Sistem

- Stabil
- Responsif
- Multitasking
- Real-time
- Terhubung Internet
- Dashboard lokal aktif

<img width="1920" height="1080" alt="Screenshot 2026-02-07 051708" src="https://github.com/user-attachments/assets/2291b812-6831-473e-a67f-d865590dd08f" />

<img width="1920" height="1080" alt="Screenshot 2026-02-07 051754" src="https://github.com/user-attachments/assets/51643b4c-6033-4e71-94dd-0f2b4670e766" />

<img width="336" height="752" align="center" alt="image" src="https://github.com/user-attachments/assets/3eb378bc-90a2-4163-bb5c-ac23cfd80a6d" />


<img width="1920" height="1080" alt="Screenshot 2026-02-07 052112" src="https://github.com/user-attachments/assets/c2e921fc-b7c8-4e97-91a0-4fce6fa2e68b" />

<img width="1920" height="1080" alt="Screenshot 2026-02-07 052208" src="https://github.com/user-attachments/assets/a98ba1a5-9aeb-4db8-94a6-ba450b34da0c" />

---

## 📚 Tujuan Pembelajaran

Proyek ini menunjukkan implementasi nyata:

- interrupt system
- manajemen proses embedded
- manajemen memori
- komunikasi jaringan IoT
- real-time multitasking
- web dashboard embedded

---

## 👩‍💻 Link Video

Youtube : 

