# C-ALGORITHM: SATIR SATIR KOD ANALİZİ VE MÜHENDİSLİK DERSLERİ
**Geliştirici:** Mustafa Can Yılmazer (@ElictryzDev)  
**Tarih:** Ekim 2026

---

## 1. DERS: `uint8_t` NEDEN `d` HARFİ OLARAK BASILDI? (INTEGER PROMOTION)

### Sorunlu Kod:
```cpp
uint8_t can = 100;
std::cout << can; // Ekrana 100 yerine 'd' yazar!
```

### Mühendislik Analizi:
1. `<cstdint>` kütüphanesindeki `uint8_t`, 8 bitlik (1 bayt) işaretsiz tam sayıdır ($0 - 255$).
2. C++ dilinde 1 baytlık veri tipi aynı zamanda `char` (karakter) tipidir.
3. `std::cout`, ekrana basılan değişkenin türüne göre aşırı yüklenmiştir (Function Overloading).
4. `std::cout` bir `uint8_t` gördüğünde onun bir "karakter" olduğunu varsayar ve ASCII tablosundaki 100. sıradaki harfi basar: **`'d'`**!

### Çözüm Yolu:
```cpp
// 1. Yol: Ekrana basarken int'e terfi ettirmek (Unary Plus veya Cast)
std::cout << +can;       // '+' operatörü değişkeni int'e terfi ettirir -> 100 basar!
std::cout << (int)can;    // Açık tür dönüşümü (Explicit Cast) -> 100 basar!

// 2. Yol: Standart tamsayı türü kullanmak
int can = 100;            // Doğrudan sayı olarak basar -> 100!
```

---

## 2. DERS: TÜRKÇE KARAKTERLER VE KOD SAYFALARI (CODE PAGES)

### Sorunlu Kod:
```cpp
std::cout << "Yeni Can Değeri: " << x << std::endl;
// Windows CMD Çıktısı: "Yeni Can De─şeri: "
```

### Mühendislik Analizi:
* Kaynak kodumuz Visual Studio ve modern editörlerde **UTF-8 (Unicode)** formatında yazılır.
* Ancak Windows Komut İstemi (CMD), geriye dönük uyumluluk için 1980'lerden kalma **OEM Kod Sayfasını (CP857)** kullanır.
* UTF-8'deki 2 baytlık `ğ` (`0xC4 0x9F`), eski kod sayfasında `─ş` grafik sembollerine denk gelir!

### Çözüm Yolu:
```cpp
#include <windows.h>

int main() {
    SetConsoleOutputCP(65001); // 65001 = UTF-8 Kod Sayfası!
    SetConsoleCP(65001);       // Klavyeden UTF-8 okuma modu
    // Artık Türkçe karakterler kusursuz çalışır!
}
```

---

## 3. DERS: MATEMATİKSEL İŞLEMLER VE KÜTÜPHANELER (`<cmath>`)

### Üretilen Kod:
```cpp
#include <cmath>

int main() {
    int x = 5;
    int sonuc = std::pow(x, 2); // 5^2 = 25
    int karekok = std::sqrt(sonuc); // sqrt(25) = 5
}
```

### Mühendislik Mantığı:
* Standart dört işlem (`+`, `-`, `*`, `/`, `%`) C++ işlemci komutlarıdır (Assembly: `ADD`, `SUB`, `IMUL`, `IDIV`).
* Ancak `pow` (üs) ve `sqrt` (karekök) işlemcinin matematiksel yardımcı birimini (FPU) kullanır ve `<cmath>` kütüphanesinde yaşar.
* C-Algorithm motorumuz artık bu kütüphaneyi otomatik olarak projeye dahil eder ve işlemleri tek tıkla görsel düğümlere bağlar!
