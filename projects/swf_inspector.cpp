#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

void inspectSwf(const std::string& path) {
    std::cout << "\n============================================\n";
    std::cout << "INSPECTING: " << path << "\n";
    std::cout << "============================================\n";

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cout << "HATA: Dosya acilamadi!\n";
        return;
    }

    char header[8];
    file.read(header, 8);
    std::string sig(header, 3);
    int version = (unsigned char)header[3];
    uint32_t length = *(uint32_t*)(header + 4);

    std::cout << "SWF Imzasi: " << sig << " (";
    if (sig == "FWS") std::cout << "Sikistirilmamis SWF";
    else if (sig == "CWS") std::cout << "ZLIB ile Sikistirilmis SWF";
    else if (sig == "ZWS") std::cout << "LZMA ile Sikistirilmis SWF";
    std::cout << ")\n";
    std::cout << "Flash Player Surumu: " << version << "\n";
    std::cout << "Dosya Baslik Boyutu: " << length << " bayt\n";

    // Dosyayi buffer'a al ve string aramasi yap
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);

    std::string content(buffer.begin(), buffer.end());

    // Anahtar kelimeleri ara
    std::vector<std::string> keywords = {
        "playerio", "PlayerIO", "quickConnect", "authenticate", 
        "developmentServer", "GameFS", "roomType", "StickRun", 
        "Manuel", "Otto", "Harman", "adobe"
    };

    std::cout << "\n--- ANAHTAR KELIME ARAMASI ---\n";
    for (const auto& kw : keywords) {
        size_t pos = 0;
        int count = 0;
        while ((pos = content.find(kw, pos)) != std::string::npos) {
            count++;
            if (count <= 3) {
                // Etrafindaki 40 karakteri goster
                size_t start = pos > 20 ? pos - 20 : 0;
                size_t len = std::min((size_t)60, content.size() - start);
                std::string snippet = content.substr(start, len);
                // Kontrol karakterlerini temizle
                for (char& c : snippet) {
                    if (c < 32 || c > 126) c = '.';
                }
                std::cout << "  [" << kw << "] Konum " << pos << ": \"" << snippet << "\"\n";
            }
            pos += kw.length();
        }
        if (count > 0) {
            std::cout << "  >> Toplam '" << kw << "' eslesmesi: " << count << "\n";
        }
    }
}

int main() {
    inspectSwf("C:\\Users\\ASUS TUF GAMING\\Desktop\\stickrun\\stickrun.swf");
    inspectSwf("C:\\Users\\ASUS TUF GAMING\\Desktop\\Stick N Trade 1.6 Summer Edition.app\\~stickrun.swf");
    return 0;
}
