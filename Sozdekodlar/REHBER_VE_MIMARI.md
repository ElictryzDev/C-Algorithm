# C-ALGORITHM MİMARİSİ, C++ VE SÖZDE KOD REHBERİ
**Geliştirici:** Mustafa Can Yılmazer (@ElictryzDev)  
**Kurum:** İstanbul Beykent Üniversitesi - Bilgisayar Programcılığı  
**Proje:** C-Algorithm Native Visual Scripting & Data Structure Engine (DirectX 11 + Dear ImGui + Modern C++17)

---

## 1. DÜRÜST VE MÜHENDİSÇE ANALİZ: DOĞRU YOLDA MISIN?

Mustafa, yazdıklarını satır satır okudum. Şimdi sana bir hocanın, bir kıdemli mühendisin vereceği en dürüst geri bildirimi veriyorum:

### Neleri %100 Doğru Kavradın?
1. **`vector<string>` Dönüş Tipi:** Evet! Fonksiyon işletim sistemindeki dosyaları gezecek ve bulduğu tüm `.calg` dosya isimlerini dinamik bir listede toplayıp sana bir paket halinde verecek.
2. **`const std::string&` Mantığı:** Değişkenin kopyasını alıp RAM'i şişirmemek (L-value reference) ve fonksiyon içinde kazara değiştirilmesini engellemek (`const`) için kullandık.
3. **`try-catch` Algısı:** Doğru. `std::filesystem` sabit diske erişir; yetki hatası veya bozuk sektör olduğunda programın işletim sistemi tarafından çöpe atılmasını (crash) engeller, hatayı güvenle yakalar.
4. **`name.empty()` Koruması:** Evet, savunmacı programlama (Defensive Programming). İsimsiz proje oluşturulamaz.
5. **`projectFilePath = folderPath + "/" + name + ".calg"`:** Dosya yolu birleştirme (Path concatenation).
6. **`::` Kapsam Çözme Operatörü (Scope Resolution):** Tam isabet! `ProjectManager::GetSavedProjectFiles`, "Git `ProjectManager` sınıfının içindeki bu fonksiyona bak" demektir.

### Nerede Yanıldın / Neyi Düzeltmeliyiz?
1. **"nextId = 1 ekmek kuyruğu mu?":** Hayır, bu bir **Benzersiz Kimlik Sayacıdır (Unique ID Counter / Auto-Increment)**. SQL'deki `PRIMARY KEY AUTO_INCREMENT` mantığıdır. Kuyruk (Queue - FIFO) veri yapısı değil, her yeni nesneye benzersiz bir TC Kimlik Numarası veren basit bir tam sayıdır. Yeni düğüm geldikçe `1, 2, 3...` artar.
2. **`60.0f, 100.0f` Değerleri:** Bunlar genişlik/yükseklik değil; düğümün ızgara (canvas) üzerindeki **(X, Y) Koordinatlarıdır**. Sol üstten ne kadar sağda ve aşağıda doğacağını belirler.
3. **Slint vs ImGui vs .hpp Tuzağı:** "Slint'e de başlayalım" dedin. **DUR! İşte tam bu an Vibe Coding tuzağıdır!** Bir yapıyı (.hpp, bellek, işaretçiler) tam oturtmadan kütüphaneden kütüphaneye atlamak mühendislik değil, dikkat dağınıklığıdır. Önce elindeki C-Algorithm'in her bir baytını iliklerine kadar hissedeceksin; sonra Slint, Qt veya Unreal senin için çocuk oyuncağı olur.

---

## 2. `.hpp` (HEADER) VS `.cpp` (SOURCE) DOSYASI FARKI

Bir restorana gittiğini düşün:
- **`.hpp` (Header - Başlık Dosyası) = YEMEK MENÜSÜDÜR.** 
  - Masadaki müşteriye (diğer kod dosyalarına) şunu der: "Bu dükkanda Köfte var, Çorba var, Ayran var. Köfte et ve tuz alır, porsiyon döner."
  - İçinde tencere kaynamaz! Yani fonksiyonun gövdesi `{ ... }` burada olmaz. Sadece ne olduğu bildirilir (Declaration).
- **`.cpp` (Source - Kaynak Dosyası) = MUTFAKTIR.**
  - Aşçının çalıştığı yerdir. Köftenin nasıl yoğrulduğu, kaç dakika piştiği, for döngüleri, matematiksel işlemler (Implementation) burada yer alır.

```cpp
// ==================== Types.hpp (MENÜ) ====================
struct FlowNode {
    int id;
    std::string title;
    void EkranaYaz(); // Sadece "böyle bir metot var" dedik, gövde yok!
};

// ==================== Types.cpp (MUTFAK) ====================
#include "Types.hpp"
void FlowNode::EkranaYaz() {
    // Mutfak burada çalışır:
    std::cout << "Dugum ID: " << id << std::endl;
}
```

---

## 3. C-ALGORITHM'DE KULLANDIĞIMIZ 8 ALTIN MİMARİ KAVRAM

### 1. `struct` (Hafıza Paketi)
- **Projede:** `FlowNode`, `FlowWire`, `StructMember`
- **Nedir:** Birbiriyle ilişkili verileri bellekte peş peşe saklayan yapıdır. `FlowNode` struct'ı olmasaydı her düğümün koordinatını, başlığını ve pinlerini 20 ayrı dizide tutmak zorunda kalırdık.

### 2. `enum class` (Tip Güvenli Liste)
- **Projede:** `NodeKind`, `TypeKind`, `PointerKind`
- **Neden düz `enum` değil?:** Düz enum'lar arka planda basit `int`'e dönüşür ve kazara sayılarla karışabilir. `enum class` güçlü tip denetimi (Strong Typing) sağlar. `NodeKind::Start` ile `TypeKind::Int32` asla birbirine atanamaz!

### 3. `std::vector<T>` (Dinamik Dizi)
- **Projede:** `nodes`, `wires`, `loopBodyStmts`
- **Bellek Mantığı:** Klasik `int dizi[10]` gibi sabit değildir. Sen düğüm ekledikçe Heap (Yığın) bellekte yer açar, doldukça boyutunu 2 katına çıkarıp elemanları taşır. $O(1)$ amortize ekleme hızına sahiptir.

### 4. `std::function` ve Lambda (`[&]`)
- **Projede:** `GenerateCppCode()` içindeki `generateChain`
- **Nedir:** Lambda, fonksiyon içinde anlık yazılan isimsiz (anonim) fonksiyondur.
- **`[&]` Yakalaması (Capture by Reference):** Fonksiyonun dışındaki tüm değişkenleri (`ss`, `wires`, `nodes`) kopyalamadan, doğrudan orijinal bellek adresleri üzerinden kullanmasını sağlar. Kopyalama maliyeti = 0!

### 5. `_popen` ve `_pclose` (İşlem / Process Borusu)
- **Projede:** Dahili Derleme Terminali
- **Nedir:** Windows işletim sistemine "Arka planda gizli bir komut satırı aç, bana onun çıktı borusunu (pipe) ver" der. İşletim sistemi `cl.exe`'nin ekrana bastığı hata ve logları bizim C++ kodumuza canlı olarak akıtır.

### 6. RAII (Resource Acquisition Is Initialization)
- **Projede:** `~Destructor()`, `std::shared_ptr`, `FILE*` yönetimi
- **Kural:** Bir nesne doğduğunda (Constructor) bellek alır; süslü parantez kapandığında (`}`) yıkıcı metodu (`Destructor`) otomatik çalışıp belleği temizler. C++'ı Java ve Python'dan ayıran en güçlü hafıza koruma zırhıdır.

### 7. ImGui Immediate Mode (Anlık Çizim)
- **Projede:** Tüm arayüz (`Workspaces.cpp`)
- **Felsefesi:** Buton bir nesne değildir! Bir if koşuludur. Ekranda her karede (saniyede 60-144 kez) piksel koordinatları yeniden hesaplanır ve DirectX 11 ekran kartı tamponuna yazılır.

---

## 4. DETAYLI SÖZDE KODLAR (PSEUDO-CODE)

### A. Proje Yükleme Algoritması (`OpenProject`)
```text
FONKSİYON OpenProject(dosyaYolu):
    Dosyayı OKUMA modunda aç
    EĞER dosya açılamadıysa:
        DURDUR ve HATA DÖNDÜR
    
    Tüm mevcut düğümleri temizle (nodes.clear())
    Tüm mevcut kabloları temizle (wires.clear())
    
    DÖNGÜ (Dosya bitene kadar her satırı oku):
        EĞER satır "[NODES]" başlığı ise:
            Bölüm modunu DUGUM yap
        EĞER satır "[WIRES]" başlığı ise:
            Bölüm modunu KABLO yap
            
        EĞER Bölüm == DUGUM:
            Satırı '|' karakterlerine göre parçala (Tokenize)
            EĞER ilk parça "BODY_STMTS" ise:
                İlgili düğümü ID ile bul
                İç döngü ifadelerini düğüme ekle
            DEĞİLSE:
                Yeni FlowNode oluştur
                ID, X, Y, Başlık, Tür değerlerini doldur
                nodes listesine ekle
                
        EĞER Bölüm == KABLO:
            Satırı parçala
            fromNode, fromPin, toNode, toPin değerlerini al
            wires listesine ekle
            
    isProjectLoaded = TRUE
    Danışman Teşhislerini Çalıştır (RunDiagnostics())
    BAŞARILI DÖNDÜR
```

### B. Otomatik C++ Kod Üretim Algoritması (`GenerateCppCode`)
```text
FONKSİYON GenerateCppCode():
    MetinAkışı (stringstream) başlat
    
    // 1. Kütüphaneleri Ekle
    MetinAkışına Yaz: "#include <iostream>", "<vector>", "<memory>"...
    
    // 2. Struct Bildirimleri (Global Alan)
    DÖNGÜ (nodes içindeki her düğüm):
        EĞER düğüm StructDef ise:
            MetinAkışına Yaz: "struct " + structName + " { ... };"
            
    // 3. main() Başlangıcı
    MetinAkışına Yaz: "int main() {"
    
    // 4. Graf Zincirini Takip Et
    ZiyaretEdilenler Kümesi = Boş
    
    FONKSİYON ZinciriTakipEt(düğümId, girinti):
        EĞER düğümId ZiyaretEdildiyse: ÇIK
        ZiyaretEdilenler.Ekle(düğümId)
        
        Düğüm = DüğümüGetir(düğümId)
        
        EĞER Düğüm WhileLoop ise:
            MetinAkışına Yaz: girinti + "while (" + condition + ") {"
            GövdeDüğümü = ÇıkışPiniHedefi(düğümId, Pin: 1)
            EĞER GövdeDüğümü VARSA:
                ZinciriTakipEt(GövdeDüğümü, girinti + 1)
            
            // Kullanıcının Inspector'dan eklediği ifadeler
            DÖNGÜ (loopBodyStmts içindeki her ifade):
                MetinAkışına Yaz: girinti + "    " + ifade + ";"
                
            MetinAkışına Yaz: girinti + "}"
            
            BitişDüğümü = ÇıkışPiniHedefi(düğümId, Pin: 2)
            EĞER BitişDüğümü VARSA:
                ZinciriTakipEt(BitişDüğümü, girinti)
                
    BaşlangıçDüğümünüBul(NodeKind::Start)
    ZinciriTakipEt(BaşlangıçDüğümü, girinti: 1)
    
    MetinAkışına Yaz: "    return 0;\n}"
    DÖNDÜR MetinAkışı.MetinHali()
```

### C. Güvenli Derleme & Çalıştırma Algoritması (`BuildAndRun`)
```text
FONKSİYON BuildAndRun(üretilenKod):
    1. "projects/_build_temp.cpp" dosyasını oluştur ve kodu içine yaz
    2. "projects/_build_run.bat" adında geçici batch dosyası oluştur:
       - Satır 1: @echo off
       - Satır 2: vcvars64.bat çağır (MSVC ortamını yükle)
       - Satır 3: cl.exe /std:c++17 _build_temp.cpp /Fe:_build_temp.exe
    3. pipe = _popen("_build_run.bat 2>&1", "r")
    
    DÖNGÜ (pipe'tan satır satır oku):
        Gelen satırı g_buildOutput tamponuna ekle
        Ekrana anında bas (ImGui canlı terminal)
        
    ÇıkışKodu = _pclose(pipe)
    
    EĞER ÇıkışKodu == 0 İSE:
        Terminal'e Yaz: "[DERLEME BAŞARILI] Program çalıştırılıyor..."
        ÇalıştırmaPipe = _popen("_build_temp.exe 2>&1", "r")
        SatırSayacı = 0
        DÖNGÜ (ÇalıştırmaPipe satır satır oku):
            Terminal'e satırı yaz
            SatırSayacı++
            EĞER SatırSayacı > 200:
                Terminal'e Yaz: "[UYARI] 200 satır aşıldı! Sonsuz döngü koruması durdurdu."
                DÖNGÜDEN ÇIK
        _pclose(ÇalıştırmaPipe)
    DEĞİLSE:
        Terminal'e Yaz: "[HATA] Derleme Başarısız! Hataları inceleyin."
```

---

## 5. SENİN İÇİN YOL HARİTASI: ÖNLİSANSIN ÖTESİNE GEÇMEK

1. **Bu Dokümanı Sakla:** Proje kökünde `Sozdekodlar/REHBER_VE_MIMARI.md` olarak kaydettik.
2. **Kodu Satır Satır Oku:** Yukarıdaki sözde kodları önüne al, yan sekmede `ProjectManager.cpp`'yi aç. Sözde kodun C++ karşılığını parmağınla takip et.
3. **Kendine Güven:** Sen bir vibe coding kullanıcısı değilsin. Vibe coder "çalıştı mı çalıştı kurcalama" der. Sen ise "Dur bir dakika, bu memory'de ne yapıyor, bunu bana anlat" dedin. Gerçek mühendis tam olarak burada başlar.
