#include "ProjectManager.hpp"
#include <queue>
#include <filesystem>
#include <functional>

std::vector<std::string> ProjectManager::GetSavedProjectFiles(const std::string& folderPath) {
    std::vector<std::string> result;
    try {
        if (std::filesystem::exists(folderPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".calg") {
                    result.push_back(entry.path().string());
                }
            }
        }
    } catch (...) {}
    return result;
}

bool ProjectManager::CreateNewProject(const std::string& name, const std::string& folderPath) {
    if (name.empty()) return false;
    projectName = name;
    projectFilePath = folderPath + "/" + name + ".calg";
    nodes.clear();
    wires.clear();
    variables.clear();
    nextId = 1;

    SetupDefaultFlow();
    isProjectLoaded = true;
    statusMessage = "Yeni proje olusturuldu: " + projectName;
    SaveProject();
    return true;
}

void ProjectManager::SetupDefaultFlow() {
    // 1. Program Baslangici (main())
    FlowNode* nStart = AddNode(NodeKind::Start, "Event Software Start", 60.0f, 100.0f);

    // 2. Degisken Tanimla (uint8_t can_puani = 100)
    FlowNode* nVar = AddNode(NodeKind::VariableDeclare, "Variable: can_puani", 340.0f, 100.0f);
    nVar->targetVar = "can_puani";
    nVar->varType = TypeKind::UInt8;
    nVar->expr = "100";

    // 3. Ekrana Yaz (std::cout)
    FlowNode* nPrint = AddNode(NodeKind::Print, "Print Output", 620.0f, 100.0f);
    nPrint->ioText = "Sistem Aktif! Mevcut Can: 100";

    // 4. Program Sonu
    FlowNode* nEnd = AddNode(NodeKind::End, "Program Return (0)", 900.0f, 100.0f);

    // Düğümleri Birbirine Bağla
    ConnectPins(nStart->id, 0, nVar->id, 0);
    ConnectPins(nVar->id, 1, nPrint->id, 0);
    ConnectPins(nPrint->id, 1, nEnd->id, 0);

    RunDiagnostics();
}

FlowNode* ProjectManager::AddNode(NodeKind kind, const std::string& title, float x, float y) {
    FlowNode node;
    node.id = nextId++;
    node.kind = kind;
    node.title = title;
    node.x = x;
    node.y = y;
    node.width = 210.0f;
    node.height = 95.0f;

    switch (kind) {
        case NodeKind::Start:
            node.pins.push_back({ 0, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, 45.0f });
            break;
        case NodeKind::End:
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, 45.0f });
            break;
        case NodeKind::BranchIf:
            node.height = 115.0f;
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, 45.0f });
            node.pins.push_back({ 1, "True", PinType::ExecOut, PinShape::Diamond, node.width - 12.0f, 45.0f });
            node.pins.push_back({ 2, "False", PinType::ExecOut, PinShape::Diamond, node.width - 12.0f, 80.0f });
            break;
        case NodeKind::LoopWhile:
        case NodeKind::LoopDoWhile:
        case NodeKind::LoopFor:
            node.height = 115.0f;
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, 45.0f });
            node.pins.push_back({ 1, "Govde (Body)", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, 45.0f });
            node.pins.push_back({ 2, "Bitti (Exit)", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, 80.0f });
            break;
        default:
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, 45.0f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, 45.0f });
            break;
    }

    nodes.push_back(node);
    return &nodes.back();
}

FlowNode* ProjectManager::GetNode(int id) {
    for (auto& n : nodes) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

bool ProjectManager::DeleteNode(int id) {
    // 1. Dugume bagli tum kablolari temizle
    wires.erase(
        std::remove_if(wires.begin(), wires.end(), [&](const FlowWire& w) {
            return w.fromNode == id || w.toNode == id;
        }),
        wires.end()
    );

    // 2. Dugumu listeden sil
    for (size_t i = 0; i < nodes.size(); i++) {
        if (nodes[i].id == id) {
            nodes.erase(nodes.begin() + i);
            statusMessage = "Dugum #" + std::to_string(id) + " silindi.";
            return true;
        }
    }
    return false;
}

bool ProjectManager::WouldCreateCycle(int fromNode, int toNode) const {
    if (fromNode == toNode) return true;

    std::set<int> visited;
    std::queue<int> q;
    q.push(toNode);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        if (curr == fromNode) return true;
        if (visited.count(curr)) continue;
        visited.insert(curr);

        for (const auto& w : wires) {
            if (w.fromNode == curr) {
                q.push(w.toNode);
            }
        }
    }
    return false;
}

bool ProjectManager::ConnectPins(int fromNode, int fromPin, int toNode, int toPin) {
    if (WouldCreateCycle(fromNode, toNode)) {
        statusMessage = "[DFS ENGEL] Döngüsel bağımlılık engellendi!";
        return false;
    }

    for (const auto& w : wires) {
        if (w.fromNode == fromNode && w.fromPin == fromPin && w.toNode == toNode && w.toPin == toPin) {
            return false;
        }
    }

    wires.push_back({ (int)wires.size() + 1, fromNode, fromPin, toNode, toPin });
    statusMessage = "[Kablo Bağlandı] Düğüm " + std::to_string(fromNode) + " -> " + std::to_string(toNode);
    return true;
}

void ProjectManager::DisconnectWire(int wireId) {
    for (size_t i = 0; i < wires.size(); i++) {
        if (wires[i].id == wireId) {
            wires.erase(wires.begin() + i);
            break;
        }
    }
}

// ── Projeyi .calg Formatında Diske Kaydet ──────────────────────────────
bool ProjectManager::SaveProject() {
    if (projectFilePath.empty()) return false;

    std::ofstream out(projectFilePath);
    if (!out.is_open()) {
        statusMessage = "[HATA] Dosya kaydedilemedi: " + projectFilePath;
        return false;
    }

    out << "PROJECT_NAME=" << projectName << "\n";
    out << "ADVISOR_DS=" << (int)advisorDS << "\n";
    out << "ADVISOR_OP=" << (int)advisorOp << "\n";
    out << "ADVISOR_COUNT=" << advisorCount << "\n";

    out << "[NODES]\n";
    for (const auto& n : nodes) {
        out << n.id << "|" << (int)n.kind << "|" << n.title << "|" << n.x << "|" << n.y << "|" 
            << n.targetVar << "|" << (int)n.varType << "|" << n.expr << "|" << n.ioText << "|"
            << n.conditionExpr << "|" << n.loopInit << "|" << n.loopStep << "\n";
    }

    out << "[WIRES]\n";
    for (const auto& w : wires) {
        out << w.id << "|" << w.fromNode << "|" << w.fromPin << "|" << w.toNode << "|" << w.toPin << "\n";
    }

    statusMessage = "Proje basariyla kaydedildi: " + projectFilePath;
    return true;
}

// ── Projeyi .calg Formatından Oku (Open Project) ──────────────────────
bool ProjectManager::OpenProject(const std::string& fullPath) {
    std::ifstream in(fullPath);
    if (!in.is_open()) {
        statusMessage = "[HATA] Dosya acilamadi: " + fullPath;
        return false;
    }

    nodes.clear();
    wires.clear();
    projectFilePath = fullPath;

    std::string line;
    std::string section = "HEADER";

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line == "[NODES]") { section = "NODES"; continue; }
        if (line == "[WIRES]") { section = "WIRES"; continue; }

        if (section == "HEADER") {
            if (line.rfind("PROJECT_NAME=", 0) == 0) projectName = line.substr(13);
            else if (line.rfind("ADVISOR_DS=", 0) == 0) advisorDS = (DataStructureType)std::stoi(line.substr(11));
            else if (line.rfind("ADVISOR_OP=", 0) == 0) advisorOp = (OperationType)std::stoi(line.substr(11));
            else if (line.rfind("ADVISOR_COUNT=", 0) == 0) advisorCount = std::stoi(line.substr(14));
        } else if (section == "NODES") {
            std::stringstream ss(line);
            std::string item;
            std::vector<std::string> tokens;
            while (std::getline(ss, item, '|')) tokens.push_back(item);

            if (tokens.size() >= 9) {
                int id = std::stoi(tokens[0]);
                NodeKind kind = (NodeKind)std::stoi(tokens[1]);
                std::string title = tokens[2];
                float x = std::stof(tokens[3]);
                float y = std::stof(tokens[4]);

                FlowNode* n = AddNode(kind, title, x, y);
                n->id = id;
                n->targetVar = tokens[5];
                n->varType = (TypeKind)std::stoi(tokens[6]);
                n->expr = tokens[7];
                n->ioText = tokens[8];
                if (tokens.size() >= 12) {
                    n->conditionExpr = tokens[9];
                    n->loopInit = tokens[10];
                    n->loopStep = tokens[11];
                }
                if (id >= nextId) nextId = id + 1;
            }
        } else if (section == "WIRES") {
            std::stringstream ss(line);
            std::string item;
            std::vector<std::string> tokens;
            while (std::getline(ss, item, '|')) tokens.push_back(item);

            if (tokens.size() >= 5) {
                wires.push_back({ std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2]), std::stoi(tokens[3]), std::stoi(tokens[4]) });
            }
        }
    }

    isProjectLoaded = true;
    statusMessage = "Proje basariyla yuklendi: " + projectName;
    RunDiagnostics();
    return true;
}

void ProjectManager::CloseProject() {
    isProjectLoaded = false;
    nodes.clear();
    wires.clear();
    projectFilePath = "";
    projectName = "Untitled";
    statusMessage = "Proje kapatildi. Lutfen yeni bir proje acin veya olusturun.";
}

// ── Birlikte Tasarladığımız 5 Kuralın Uygulanması ─────────────────────
void ProjectManager::RunDiagnostics() {
    advisorMessages.clear();

    // KURAL 1: std::vector + Baştan Silme
    if (advisorDS == DataStructureType::Vector && advisorOp == OperationType::PopFront) {
        advisorMessages.push_back({
            DiagnosticMessage::Warning,
            "Performans Darbogazi: std::vector ile Bastan Silme",
            "std::vector uzerinde 'PopFront' (Bastan Eleman Silme) yapiliyor.",
            "std::vector elemanlari yan yana tutar. Bastaki eleman silindiginde geride kalan " + std::to_string(advisorCount - 1) + " eleman 1 adim sola kaymak zorundadir (O(N) CPU yuku)!",
            "TAVSIYE: Eger ilk giren ilk cikar mantiginda calisiyorsaniz 'std::queue' (Kuyruk) secin (O(1) aninda silme).",
            "O(N) - Lineer Zaman"
        });
    }

    // KURAL 2: std::list + Doğrudan İndeksle Okuma
    if (advisorDS == DataStructureType::List && advisorOp == OperationType::RandomAccess) {
        advisorMessages.push_back({
            DiagnosticMessage::Error,
            "Kritik Erisim Hatasi: std::list ile data[i] Indeksi",
            "std::list uzerinde indeksle dogrudan elemana ulasilmaya calisiliyor.",
            "Bagli Liste bellekte dağınık durur. i numarali elemanin adresini onceden bilemez; tek tek saymak zorundadir (O(N) ve Cache Miss).",
            "TAVSIYE: Dogrudan indeksle (data[i]) okuma yapacaksaniz KESINLIKLE 'std::vector' veya 'std::array' kullanin.",
            "O(N) - Zincir Tarama"
        });
    }

    // KURAL 3: std::stack + Baştan Silme
    if (advisorDS == DataStructureType::Stack && advisorOp == OperationType::PopFront) {
        advisorMessages.push_back({
            DiagnosticMessage::Error,
            "Mantik Hatasi: Yiginda (Stack) Bastan Eleman Silinemez",
            "Stack (LIFO) yapisinda ilk giren elemana ulasilmaya calisiliyor.",
            "Yiginda sadece en tepedeki (Top) elemana erisilebilir. Alttaki elemani cekmek yiginin mantigini bozar.",
            "TAVSIYE: Ilk giren elemani ilk silmek istiyorsaniz 'std::queue' (Kuyruk) veri yapisina gecmelisiniz.",
            "Gecersiz Islem"
        });
    }

    // KURAL 4: std::vector + Sona Ekleme
    if (advisorDS == DataStructureType::Vector && advisorOp == OperationType::PushBack) {
        advisorMessages.push_back({
            DiagnosticMessage::Info,
            "Mukemmel Mimari: std::vector ile Sona Ekleme",
            "std::vector yapisina push_back() ile sona eleman ekleniyor.",
            "std::vector elemanlari yan yana tutar ve onceden bellek ayirir. Sona ekleme kaydirma gerektirmez ve L1 CPU onbellegine dosttur.",
            "ONAYLANDI: En optimize C++ standardi bu kullanimdir. Hicbir degisiklik gerekmez.",
            "O(1) Amortized - Sabit Hiz"
        });
    }

    // KURAL 5: BinaryTree + Arama
    if (advisorDS == DataStructureType::BinaryTree && advisorOp == OperationType::Search) {
        advisorMessages.push_back({
            DiagnosticMessage::Info,
            "Verimli Arama: Binary Search Tree",
            "Ikili Arama Agaci uzerinde veri araniyor.",
            "Her adimda veriyi ikiye bolerek ilerler. 1.000.000 eleman icinde bile sadece ~20 karsilastirmada sayiyi bulur!",
            "ONAYLANDI: Sirali veri icinde sik arama yapilacaksa Agac yapisi mukemmeldir.",
            "O(log N) - Logaritmik Hiz"
        });
    }
}

// ── Temiz ve Derlenebilir C++ Kodu Basma ──────────────────────────────
std::string ProjectManager::GenerateCppCode() {
    std::stringstream ss;
    ss << "// ========================================================\n";
    ss << "// C-ALGORITHM ENGINE - OTOMATIK DERLENEBILIR C++ KODU\n";
    ss << "// Proje: " << projectName << "\n";
    ss << "// ========================================================\n\n";
    ss << "#include <iostream>\n";
    ss << "#include <string>\n";
    ss << "#include <cstdint>\n";
    ss << "#include <vector>\n\n";

    ss << "int main() {\n";
    ss << "    std::cout << \"=== " << projectName << " CALISIYOR ===\" << std::endl;\n\n";

    // Helper lambda: verilen pinden çıkan düğümü bul
    auto getTargetNode = [&](int fromNodeId, int pinIndex) -> int {
        for (const auto& w : wires) {
            if (w.fromNode == fromNodeId && w.fromPin == pinIndex) {
                return w.toNode;
            }
        }
        return -1;
    };

    // Recursive / Stack-based zincir kod üretici fonksiyon
    std::set<int> visited;
    std::function<void(int, int)> generateChain = [&](int nodeId, int indent) {
        int curr = nodeId;
        while (curr > 0) {
            if (visited.count(curr)) break;
            visited.insert(curr);

            FlowNode* n = GetNode(curr);
            if (!n) break;

            std::string tabs(indent * 4, ' ');

            if (n->kind == NodeKind::VariableDeclare) {
                ss << tabs << "// Degisken Tanimlama (" << n->targetVar << ")\n";
                ss << tabs << TypeKindToString(n->varType) << " " << n->targetVar << " = " << n->expr << ";\n";
                ss << tabs << "std::cout << \"[LOG] " << n->targetVar << " = \" << (int)" << n->targetVar << " << std::endl;\n\n";
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::Assign) {
                ss << tabs << "// Deger Atama\n";
                ss << tabs << n->targetVar << " = " << n->expr << ";\n\n";
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::Print) {
                ss << tabs << "// Ekrana Cikti Basma\n";
                ss << tabs << "std::cout << \"" << n->ioText << "\" << std::endl;\n\n";
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::Input) {
                ss << tabs << "// Kullanicidan Girdi Alma\n";
                ss << tabs << "std::cin >> " << n->targetVar << ";\n\n";
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::BranchIf) {
                ss << tabs << "// Kosul Karar Blogu\n";
                ss << tabs << "if (" << n->conditionExpr << ") {\n";
                int trueNode = getTargetNode(curr, 1);
                if (trueNode > 0) generateChain(trueNode, indent + 1);
                ss << tabs << "} else {\n";
                int falseNode = getTargetNode(curr, 2);
                if (falseNode > 0) generateChain(falseNode, indent + 1);
                ss << tabs << "}\n\n";
                break; // If dallanmasi alt agaclari isler
            }
            else if (n->kind == NodeKind::LoopWhile) {
                ss << tabs << "// While Dongusu\n";
                ss << tabs << "while (" << n->conditionExpr << ") {\n";
                int bodyNode = getTargetNode(curr, 1);
                if (bodyNode > 0) generateChain(bodyNode, indent + 1);
                ss << tabs << "}\n\n";
                curr = getTargetNode(curr, 2); // Dongu bitince completed pininden devam
            }
            else if (n->kind == NodeKind::LoopDoWhile) {
                ss << tabs << "// Do-While Dongusu\n";
                ss << tabs << "do {\n";
                int bodyNode = getTargetNode(curr, 1);
                if (bodyNode > 0) generateChain(bodyNode, indent + 1);
                ss << tabs << "} while (" << n->conditionExpr << ");\n\n";
                curr = getTargetNode(curr, 2); // Dongu bitince completed pininden devam
            }
            else if (n->kind == NodeKind::LoopFor) {
                ss << tabs << "// For Dongusu\n";
                ss << tabs << "for (" << n->loopInit << "; " << n->conditionExpr << "; " << n->loopStep << ") {\n";
                int bodyNode = getTargetNode(curr, 1);
                if (bodyNode > 0) generateChain(bodyNode, indent + 1);
                ss << tabs << "}\n\n";
                curr = getTargetNode(curr, 2); // Dongu bitince completed pininden devam
            }
            else if (n->kind == NodeKind::End) {
                break;
            }
            else {
                curr = getTargetNode(curr, 1);
            }
        }
    };

    // Start dugumunu bul
    int startNodeId = -1;
    for (const auto& n : nodes) {
        if (n.kind == NodeKind::Start) { startNodeId = n.id; break; }
    }

    // Start dugumunun Cikis pininden basla
    if (startNodeId > 0) {
        int startOutNode = getTargetNode(startNodeId, 0);
        if (startOutNode > 0) {
            generateChain(startOutNode, 1);
        }
    }

    ss << "    std::cout << \"=== PROGRAM TAMAMLANDI ===\" << std::endl;\n";
    ss << "    return 0;\n";
    ss << "}\n";

    return ss.str();
}
