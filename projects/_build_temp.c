/* ======================================================== */
/* C-ALGORITHM ENGINE - SAF ANSI C (C99/C11) KODU           */
/* Proje: Test Alanı
/* Açıklama: C++ ek yükleri (iostream, vtable, RTTI)       */
/*           olmadan, saf libc ve stdio ile en hafif C kodu */
/* ======================================================== */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <windows.h>

int main(void) {
    /* Windows Konsolunu UTF-8 Moduna Al (Türkçe Karakter Desteği) */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printf("=== Test Alanı (C99 MODU) CALISIYOR ===\n\n");

    /* Değişken Tanımlama (can) */
    int can = 100;
    printf("[LOG] can = %d\n", can);

    int eklenecek_can = 0;
    int güncel_can_degeri = 0;
    /* Ekrana Yazdırma (printf) */
    printf("Sistem Aktif! Mevcut Can: 100\n");

    /* For Döngüsü (C99) */
    for (int i = 0; i < 10; i++) {
        /* Klavyeden Girdi Alma (scanf) */
        if (scanf("%d", &eklenecek_can) != 1) {
            /* C Dili Akış Kalkanı (Tampon Belleği Temizle) */
            int _c; while ((_c = getchar()) != '\n' && _c != EOF);
        }

        /* Sabit Boyutlu ANSI C Dizisi */
        int sabitDizi[5] = {0};
        sabitDizi[0] = 100;
        printf("[ARRAY] sabitDizi[%d] = %d\n", 0, sabitDizi[0]);

        /* Ekrana Yazdırma (printf) */
        printf("Merhaba C-Algorithm!\n");

    }

    printf("=== C PROGRAMI BASARIYLA TAMAMLANDI ===\n");
    return 0;
}
