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
#include <windows.h>

int main() {
    // Windows Konsolunu UTF-8 Moduna Al (Turkce Karakter Destegi)
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    std::cout << "=== Algoritma_Test_01 CALISIYOR ===" << std::endl;

    // Degisken Tanimlama (can)
    int can = 100;
    std::cout << "[LOG] can = " << can << std::endl;

    int sonuc = 0;
    int x = 50;
    // Matematiksel Islem: sonuc
    sonuc = can + x;
    std::cout << "[ISLEM] sonuc = " << +can << " + " << +x << " -> " << +sonuc << std::endl;

    // Ekrana Cikti Basma
    std::cout << "Toplanan X ve Can değişkenlerinin toplam değerleri: " << std::endl;

    std::cout << "=== PROGRAM TAMAMLANDI ===" << std::endl;
    return 0;
}
