// ========================================================
// C-ALGORITHM ENGINE - OTOMATIK DERLENEBILIR C++ KODU
// Proje: Test Alanı
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

    std::cout << "=== Test Alanı CALISIYOR ===" << std::endl;

    // Degisken Tanimlama (can)
    int can = 100;
    std::cout << "[LOG] can = " << can << std::endl;

    int eklenecek_can = 0;
    int güncel_can_degeri = 0;
    // Ekrana Cikti Basma
    std::cout << "Sistem Aktif! Mevcut Can: 100" << std::endl;

    // For Dongusu
    for (int i = 0; i < 10; i++) {
        // Kullanicidan Girdi Alma (std::cin >> eklenecek_can)
        std::cin >> eklenecek_can;

        // Matematiksel Islem: güncel_can_degeri
        güncel_can_degeri = can + eklenecek_can;
        std::cout << "[ISLEM] güncel_can_degeri = " << +can << " + " << +eklenecek_can << " -> " << +güncel_can_degeri << std::endl;

        // Ekrana Cikti Basma
        std::cout << "Merhaba C-Algorithm!" << std::endl;

    }

    std::cout << "=== PROGRAM TAMAMLANDI ===" << std::endl;
    return 0;
}
