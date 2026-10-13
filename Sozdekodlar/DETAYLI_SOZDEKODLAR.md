# MODÜL VE DOSYA SÖZDE KOD REHBERİ (DETAYLI ANALİZ)
**Hazırlayan:** LibertyElic (Yazılım Mimarisi Mentoru)  
**Öğrenci & Geliştirici:** Mustafa Can Yılmazer (@ElictryzDev)  
**Tarih:** Ekim 2026

---

## 1. `src/main.cpp` — OYUN MOTORU GİBİ ÇALIŞAN KALBİN SÖZDE KODU

Çoğu insan masaüstü programlarının "tıklandıkça çalışan" pasif şeyler olduğunu sanır. Oysa `C-Algorithm`, Unreal Engine veya bir FPS oyunu gibi saniyede 60-144 defa baştan hesaplanan bir **Oyun Döngüsü (Game Loop)** üzerinde yaşar!

### Sözde Kod:
```text
FONKSİYON WinMain (veya main) [Windows Giriş Kapısı]:
    1. Windows Penceresi Oluştur (DirectX için HWND oluştur)
    2. DirectX 11 Grafik Donanımını Başlat:
       - g_pd3dDevice (Ekran kartına komut veren sürücü)
       - g_pSwapChain (Monitör ile ekran kartı arasındaki çift tamponlama - Double Buffering)
       - g_mainRenderTargetView (Çizim yapılacak bellek alanı)
    3. Dear ImGui Başlat:
       - ImGui::CreateContext()
       - DirectX11 ve Win32 sürücülerini bağla
    4. ProjectManager Nesnesini Yarat:
       - pm (Tüm veritabanımız, düğümlerimiz, ayarlarımız RAM'de doğar)
    5. Varsayılan Temayı Uygula (OLED Dark Theme - Turkuaz ve Gece Mavisi)

    // ANA MOTOR DÖNGÜSÜ (GAME LOOP - Saniyede 60+ Kez Döner)
    DÖNGÜ (Pencere Kapatılmadığı Sürece):
        A. Windows Olaylarını Dinle (Mouse tıklandı mı, klavyeye basıldı mı?)
           EĞER Olay varsa -> Windows'a bildir (DispatchMessage)
        
        B. ImGui Yeni Kare Başlat (New Frame):
           - Ekran kartı saatini sıfırla
           - Fare koordinatlarını al
           
        C. MİMARİ PANEL ÇİZİMLERİ (Senin Ekranda Gördüğün Her Şey):
           EĞER Proje Yüklü Değilse:
               Giriş Ekranını Çiz (RenderProjectGatingModal)
           DEĞİLSE:
               Üst Menü Çubuğunu Çiz (Tab Bar: Akış Şeması, Danışman, Kod, Ayarlar)
               EĞER Aktif Sekme == "Akış Şeması":
                   Sol Panel: Graf Tuvalini Çiz (RenderFlowGraphWorkspace)
                   Sağ Panel: Seçili Düğüm Özelliklerini Çiz (RenderInspectorPanel)
               EĞER Aktif Sekme == "Veri Yapısı Danışmanı":
                   Danışman Analizlerini Çiz (RenderAdvisorWorkspace)
               EĞER Aktif Sekme == "Canlı C++ Kodu":
                   Kod Görüntüleyici ve Terminali Çiz (RenderCodegenWorkspace)
                   
        D. EKRAN KARTINA GÖNDERME (DirectX 11 Render):
           - Ekranı arka plan rengiyle temizle (ClearRenderTargetView)
           - ImGui çizim komutlarını DirectX komutlarına çevir
           - g_pSwapChain->Present(1, 0) [Monitöre 'Görüntüyü Göster' sinyali ver (V-Sync)]

    6. TEMİZLİK (RAII & Kapanış):
       - DirectX tamponlarını serbest bırak
       - Pencereyi kapat, işletim sistemine '0' (Başarılı) dön.
```

---

## 2. `src/Core/Types.hpp` — VERİ MODELİ VE TİP SİSTEMİ SÖZDE KODU

Bu dosya motorun omurgasıdır. Eğer bu dosyayı yanlış tasarlarsan tüm bina çöker.

### Sözde Kod:
```text
TÜR TANIMLARI:
    TypeKind: [Int32, Int64, Float, Double, Bool, UInt8, UInt16, UInt32, UInt64, String]
    PointerKind: [RawValue (Stack), SharedPtr (Heap/RAII), UniquePtr (Heap/Tekil)]
    NodeKind: [Start, End, VariableDeclare, Assign, Print, Input, BranchIf, 
               LoopWhile, LoopDoWhile, LoopFor, SwitchCase, StructDef,
               VectorOp, ListOp, StackOp, QueueOp, ArrayOp]

YAPI FlowPin (Bağlantı Soketi):
    id: Sayı
    name: Metin ("Giris", "Cikis", "Govde", "Bitti")
    type: ExecIn (Beyaz Üçgen Giriş) VEYA ExecOut (Çıkış) VEYA Data
    
YAPI FlowNode (Graf Düğümü - Atomik Birim):
    id: Benzersiz Sayı
    kind: NodeKind
    title: Metin ("While Loop", "std::vector", "Değişken")
    x, y: Koordinatlar (Izgara üzerindeki konumu)
    
    // Değişken ve Pointer Verileri:
    targetVar: Değişken Adı ("sayac", "oyuncu")
    varType: TypeKind (UInt8, Int32...)
    expr: Başlangıç Değeri ("1", "100")
    pointerKind: PointerKind
    isConst: Mantıksal (Değiştirilemez mi?)
    
    // Döngü Özellikleri (Seninle Çözdüğümüz Hayati Alanlar):
    conditionExpr: Metin ("sayac < 10")
    loopBodyStmts: Metin Dizisi (["sayac++", "std::cout << ..."])
    
    // Veri Yapıları Özellikleri (Danışman ve Kod Motoru İçin):
    dsName: Veri Yapısı Adı ("bagliListe", "sayilar")
    dsOpType: İşlem Türü ("push_back", "pop", "push")
    dsOpArg: İşlem Parametresi ("42", "10")

YAPI FlowWire (Kablo - Veri/Akış Köprüsü):
    id: Benzersiz Sayı
    fromNode, fromPin: Hangi düğümün hangi bacağından çıktı?
    toNode, toPin: Hangi düğümün hangi bacağına girdi?
```

---

## 3. `src/UI/Workspaces.cpp` — EKRAN VE ÇİZİM MATEMATİĞİ SÖZDE KODU

### Fare Odaklı Zoom (Pivot Kamera) Matematiği:
Bir haritada mouse'u nereye koyup tekerleği çevirirsen oranın büyümesi gerekir. Eğer bunu yapmazsan ekran kayar ve kullanıcı kaybolur!
```text
FONKSİYON KameraZoomHesapla(fareEkranPozisyonu, tekerlekHareketi):
    EskiZoom = g_flowZoomLevel
    
    // 1. Farenin dünyadaki (ızgaradaki) gerçek yerini bul:
    FareDunyaX = (fareEkranPozisyonu.x - EkranBaslangici.x - g_flowPanOffset.x) / EskiZoom
    FareDunyaY = (fareEkranPozisyonu.y - EkranBaslangici.y - g_flowPanOffset.y) / EskiZoom
    
    // 2. Yeni zoom katsayısını hesapla:
    EĞER tekerlek yukarı: YeniZoom = EskiZoom * 1.1 (Büyüt)
    EĞER tekerlek aşağı:  YeniZoom = EskiZoom * 0.9 (Küçült)
    YeniZoom sınırla: 0.3 ile 2.5 arasında tut
    
    // 3. Farenin baktığı dünya noktasını ekranda SABİTLE:
    g_flowPanOffset.x = fareEkranPozisyonu.x - EkranBaslangici.x - (FareDunyaX * YeniZoom)
    g_flowPanOffset.y = fareEkranPozisyonu.y - EkranBaslangici.y - (FareDunyaY * YeniZoom)
    
    g_flowZoomLevel = YeniZoom
```

### Canlı Kod Denetimi (Inline Linter) Mantığı:
Kullanıcı derle butonuna basmadan önce hatayı yüzüne söyleyen koruma kalkanı!
```text
FONKSİYON InspectorMetinKontrol(kullanicininYazdigiMetin):
    EĞER metin içinde "endl" GEÇİYORSA VE "std::endl" GEÇMİYORSA:
        Kırmızı Yazı Bas: "[!] 'endl' yerine 'std::endl' yazın"
        
    EĞER metin içinde "cout" GEÇİYORSA VE "std::cout" GEÇMİYORSA:
        Kırmızı Yazı Bas: "[!] 'cout' yerine 'std::cout' yazın"
        
    EĞER metin sonu ';' İLE BİTİYORSA:
        Turuncu Yazı Bas: "[!] Sona ';' koyma — sistem otomatik ekliyor"
```

---

## 4. DANIŞMAN MOTORU (ADVISOR) MANTIĞI: $O(1)$ VS $O(N)$

Bilgisayar programcılığında seni öne geçirecek en kritik konu **Zaman Karmaşıklığıdır (Big-O)**. C-Algorithm'in içindeki kurallar bunu öğretir:

1. **`std::vector` Başından Eleman Silmek (`pop_front`):**
   - Karmaşıklık: $O(N)$ (Felaket!)
   - Neden? Çünkü ilk eleman silinince arkadaki 100.000 elemanın hepsi RAM'de 1 adım sola kaymak zorundadır.
   - Reçete: `std::list` (Bağlı Liste) veya `std::deque` kullan! Sadece pointer yönü değişir, anında biter ($O(1)$).

2. **`std::list` İle Rastgele Elemana Erişmek (`[5000]` İndeks):**
   - Karmaşıklık: $O(N)$ (Yavaş!)
   - Neden? Bağlı listede bellek ardışık değildir. 5000. elemana gitmek için baştan başlayıp 5000 tane düğümün adresini tek tek ziyaret etmek zorundasın.
   - Reçete: `std::vector` kullan! RAM'de yan yana dururlar, `baslangic + (5000 * sizeof(int))` ile tek işlemde erişir ($O(1)$).
