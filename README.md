# ⚡ C-Algorithm — Visual Scripting & Data Structure Advisor (Native IDE)

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/Language-C%2B%2B17-00599C.svg?logo=c%2B%2B)](https://isocpp.org/)
[![DirectX 11](https://img.shields.io/badge/Graphics-DirectX_11-green.svg)](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-graphics)
[![Dear ImGui](https://img.shields.io/badge/GUI-Dear_ImGui-orange.svg)](https://github.com/ocornut/imgui)
[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-Donate-yellow.svg)](https://buymeacoffee.com/elictryzn)

> **Modern, Yüksek Performanslı Görsel Algoritma & Veri Yapıları Mühendislik IDE'si**  
> *Sıfır Web Teknolojisi, %100 Yerel C++17 / DirectX 11, 144+ FPS Donanım Hızlandırmalı Çizim.*  
> *Geliştirici:* **Mustafa Can Yılmazer ([@ElictryzDev](https://github.com/ElictryzDev))** — İstanbul Beykent Üniversitesi, Bilgisayar Programcılığı.

---

## 🌍 Dil Seçimi / Language Selection
- [Türkçe Dokümantasyon](#-türkçe-özet)
- [English Documentation](#-english-overview)

---

## 🇹🇷 Türkçe Özet

### 🚀 Proje Vizyonu ve Amacı
**C-Algorithm**, algoritma öğrenen öğrencilerin ve yazılım mühendislerinin klasik, hantal akış şeması araçları (Flowgorithm vb.) yerine; modern oyun motoru düğüm mimarilerini (Unreal Engine Blueprints tarzı) ve uluslararası akış şeması standartlarını (ISO 5807) bir arada sunan **yerel bir C++ masaüstü geliştirme ortamıdır**. 

Kullanıcı akış diyagramını görsel olarak çizerken; motor arka planda eşzamanlı olarak:
1. **Veri Yapıları Danışmanı (Data Structure Advisor):** Bellek yerleşimi (Stack vs. Heap), önbellek isabet oranı (Cache Locality) ve Big-$O$ asimptotik karmaşıklığını ($O(1)$, $O(N)$, $O(N \log N)$) canlı analiz eder.
2. **Kusursuz Kod Üretimi (Code Generation):** Çizilen şemaları anında bağımsız, derlenebilir ve hatasız modern C++17 koduna dönüştürür.
3. **Akıllı Akış Güvenliği (Stream Shield):** `cin >> sayi` gibi veri girişlerinde kullanıcı metin girdiğinde sonsuz döngüye düşmeyi engelleyen `cin.clear()` kalkanını otomatik enjekte eder.

### 📐 Standart Algoritma Şemaları (Flowchart Shapes)
- **Hap / Elips (Pill):** `Yazılımın Başlangıcı (main)`, `Yazılımın Sonlanışı (return 0)` ve `Döngü Sonu (})`.
- **Paralelkenar (Parallelogram):** `Konsola Yazdır (std::cout)` ve `Klavyeden Girdi (std::cin)`.
- **Eşkenar Dörtgen / Baklava (Diamond):** `Koşul / Karar (if - else)`. Sağ çıkış: **Evet (True)**, Alt çıkış: **Hayır (False)**.
- **Altıgen (Hexagon):** `For Döngüsü`, `While Döngüsü`, `Do-While Döngüsü`. Gövde çıkışı ile döngü içine dallanır, `Döngü Sonu` şeması ile dış akışa geri döner.
- **Dikdörtgen (Rectangle):** `Değişken Tanımla`, `Değer Ata`, `Matematiksel İşlem`, `Struct Yapısı`, `std::vector`, `std::list`, `std::stack`, `std::queue`, `Array`.

### 🎨 Arayüz ve Temalar
- **Açık Pastel Neon (Varsayılan):** Şampanya-lavanta zemin (`#F7F5FA`), leylak-ametist ızgara ve petrol yeşili / açık pembe parlak neon vurgular. Gözü yormaz, modern ve berraktır.
- **Koyu Cyberpunk Neon:** Obsidiyen siyahı zemin, turkuaz ve eflatun ışıltılar.

### 🛠️ Derleme ve Çalıştırma
```cmd
compile.bat
C-Algorithm.exe
```

---

## 🇬🇧 English Overview

### 🚀 Vision & Architecture
**C-Algorithm** is a high-performance, native C++ desktop IDE designed to bridge the gap between traditional flowchart design and modern systems programming. Instead of bulky, outdated academic flowchart tools, C-Algorithm offers smooth node canvas navigation, rich C++17 type systems, real-time memory analysis, and robust code generation.

### 🌟 Key Engineering Highlights
1. **Interactive Node Canvas:** Smooth pan & zoom (0.6x to 1.6x), cubic Bezier connection lines with directional arrowheads (▶) and connection pins (•).
2. **Dedicated Loop End Schema:** Clean execution graph modeling where loop bodies clearly terminate into a `Loop End` node (`}`), continuing onward with standard sequential execution.
3. **Resilient Stream Input:** Protects against `std::cin` stream `failbit` lockups when non-numeric values are passed to numeric variables.
4. **Data Structure Diagnostics:** Explains memory layouts, LIFO/FIFO mechanics, contiguous vs pointer-based storage, and asymptotic performance.

---

## ☕ Destek & Sponsorluk (Support & Sponsorship)

C-Algorithm tamamen bağımsız, açık kaynaklı bir eğitim ve mühendislik girişimidir. Geliştirmeyi desteklemek, yeni özelliklerin eklenmesini hızlandırmak veya bir kahve ısmarlamak isterseniz:

- ☕ **Buy Me a Coffee:** [buymeacoffee.com/elictryzn](https://buymeacoffee.com/elictryzn)
- 🐙 **GitHub:** [@ElictryzDev](https://github.com/ElictryzDev)

---

## 📜 Lisans (License)

Bu proje **[Apache License 2.0](LICENSE)** kapsamında lisanslanmıştır. Fikri mülkiyet ve atıf detayları için [NOTICE](NOTICE) dosyasına bakınız.  
Copyright (C) 2026 Mustafa Can Yılmazer (@ElictryzDev). All rights reserved.
