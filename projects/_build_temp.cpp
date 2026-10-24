// ========================================================
// C-ALGORITHM ENGINE - OTOMATIK DERLENEBILIR C++ KODU
// Proje: Algoritma_Test_01
// ========================================================

#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <array>
#include <memory>
#include <cmath>

int main() {
    std::cout << "=== Algoritma_Test_01 CALISIYOR ===" << std::endl;

    // Degisken Tanimlama (can)
    uint8_t can = 100;
    std::cout << "[LOG] can = " << can << std::endl;

    uint8_t x = 50;
    uint8_t sonuc = 0;
    // Matematiksel Islem: sonuc
    sonuc = can + x;
    std::cout << "[ISLEM] sonuc = " << can << " + " << x << " -> " << sonuc << std::endl;

    // Ekrana Cikti Basma
    std::cout << "Toplanan X ve Can değişkenlerinin toplam değerleri:  " << sonuc << std::endl;

    std::cout << "=== PROGRAM TAMAMLANDI ===" << std::endl;
    return 0;
}
