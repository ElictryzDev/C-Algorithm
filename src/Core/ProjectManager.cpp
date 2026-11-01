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
    bool isEn = (currentLanguage == AppLanguage::English);

    // 1. Program Baslangici (main())
    FlowNode* nStart = AddNode(NodeKind::Start, isEn ? "Program Start" : "Yazılımın Başlangıcı", 60.0f, 100.0f);

    // 2. Degisken Tanimla (uint8_t can_puani = 100)
    FlowNode* nVar = AddNode(NodeKind::VariableDeclare, isEn ? "Variable: can_puani" : "Değişken: can_puani", 340.0f, 100.0f);
    nVar->targetVar = "can_puani";
    nVar->varType = TypeKind::UInt8;
    nVar->expr = "100";

    // 3. Ekrana Yaz (std::cout)
    FlowNode* nPrint = AddNode(NodeKind::Print, isEn ? "Print Output" : "Konsola Yazdır", 620.0f, 100.0f);
    nPrint->ioText = isEn ? "System Online! Current Health: 100" : "Sistem Aktif! Mevcut Can: 100";

    // 4. Program Sonu
    FlowNode* nEnd = AddNode(NodeKind::End, isEn ? "Program End (return 0)" : "Yazılımın Sonlanışı", 900.0f, 100.0f);

    // Şemaları Birbirine Bağla
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
    node.width = 230.0f;
    node.height = 90.0f;

    switch (kind) {
        case NodeKind::Start:
            node.width = 190.0f;
            node.height = 65.0f;
            node.pins.push_back({ 0, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 14.0f, node.height * 0.5f });
            break;
        case NodeKind::End:
            node.width = 190.0f;
            node.height = 65.0f;
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 14.0f, node.height * 0.5f });
            break;
        case NodeKind::BranchIf:
            node.width = 240.0f;
            node.height = 110.0f;
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 14.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "True", PinType::ExecOut, PinShape::Diamond, node.width - 14.0f, node.height * 0.5f });
            node.pins.push_back({ 2, "False", PinType::ExecOut, PinShape::Diamond, node.width * 0.5f, node.height - 12.0f });
            break;
        case NodeKind::LoopWhile:
        case NodeKind::LoopDoWhile:
        case NodeKind::LoopFor:
            node.width = 240.0f;
            node.height = 100.0f;
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 14.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Govde (Body)", PinType::ExecOut, PinShape::Triangle, node.width - 14.0f, node.height * 0.35f });
            node.pins.push_back({ 2, "Bitti (Exit)", PinType::ExecOut, PinShape::Triangle, node.width - 14.0f, node.height * 0.75f });
            break;
        case NodeKind::Input:
            node.width = 230.0f;
            node.height = 80.0f;
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 14.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 14.0f, node.height * 0.5f });
            break;
        case NodeKind::Print:
            node.width = 240.0f;
            node.height = 85.0f;
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 14.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 14.0f, node.height * 0.5f });
            break;
        case NodeKind::SwitchCase:
            node.width = 240.0f;
            node.height = 150.0f;
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, 45.0f });
            node.pins.push_back({ 1, "Case 1", PinType::ExecOut, PinShape::Diamond, node.width - 12.0f, 45.0f });
            node.pins.push_back({ 2, "Case 2", PinType::ExecOut, PinShape::Diamond, node.width - 12.0f, 75.0f });
            node.pins.push_back({ 3, "Default", PinType::ExecOut, PinShape::Diamond, node.width - 12.0f, 105.0f });
            node.pins.push_back({ 4, "Bitti (Exit)", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, 135.0f });
            break;
        case NodeKind::StructDef:
            node.width = 250.0f;
            node.height = 120.0f;
            node.structName = "Oyuncu";
            node.hasDestructor = true;
            node.destructorMsg = "Oyuncu nesnesi bellekten silindi (RAII Temizlendi)!";
            node.structMembers = {
                { "can", TypeKind::Int32, "100", true },
                { "puan", TypeKind::Int32, "0", true },
                { "isim", TypeKind::String, "\"Oyuncu_1\"", true }
            };
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, node.height * 0.5f });
            break;
        case NodeKind::VectorOp:
            node.width = 230.0f;
            node.height = 100.0f;
            node.dsName = "sayilar";
            node.vectorName = "sayilar";
            node.dsOpType = "push_back";
            node.vectorOpType = "push_back";
            node.dsOpArg = "10";
            node.vectorOpArg = "10";
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, node.height * 0.5f });
            break;
        case NodeKind::ListOp:
            node.width = 230.0f;
            node.height = 100.0f;
            node.dsName = "bagliListe";
            node.dsOpType = "push_back";
            node.dsOpArg = "42";
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, node.height * 0.5f });
            break;
        case NodeKind::StackOp:
            node.width = 230.0f;
            node.height = 100.0f;
            node.dsName = "yiginStack";
            node.dsOpType = "push";
            node.dsOpArg = "50";
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, node.height * 0.5f });
            break;
        case NodeKind::QueueOp:
            node.width = 230.0f;
            node.height = 100.0f;
            node.dsName = "kuyrukQueue";
            node.dsOpType = "push";
            node.dsOpArg = "99";
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, node.height * 0.5f });
            break;
        case NodeKind::ArrayOp:
            node.width = 230.0f;
            node.height = 100.0f;
            node.dsName = "sabitDizi";
            node.arraySize = 5;
            node.arrayIndex = 0;
            node.dsOpArg = "100";
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, node.height * 0.5f });
            break;
        case NodeKind::MathOp:
            node.width = 230.0f;
            node.height = 90.0f;
            node.mathTarget = "sonuc";
            node.mathLeft = "a";
            node.mathOp = "+";
            node.mathRight = "b";
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, node.height * 0.5f });
            break;
        default:
            node.pins.push_back({ 0, "Giris", PinType::ExecIn, PinShape::Triangle, 12.0f, node.height * 0.5f });
            node.pins.push_back({ 1, "Cikis", PinType::ExecOut, PinShape::Triangle, node.width - 12.0f, node.height * 0.5f });
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
        // Govde ifadelerini ayri satir olarak yaz (While/DoWhile icin)
        if (!n.loopBodyStmts.empty()) {
            out << "BODY_STMTS|" << n.id;
            for (const auto& s : n.loopBodyStmts)
                out << "|" << s;
            out << "\n";
        }
        // MathOp alanlarini ozel satir olarak yaz
        if (n.kind == NodeKind::MathOp) {
            out << "MATH_OP|" << n.id << "|" << n.mathTarget << "|" << n.mathLeft << "|" << n.mathOp << "|" << n.mathRight << "\n";
        }
        // Print modu icin ozel satir
        if (n.kind == NodeKind::Print) {
            out << "PRINT_MODE|" << n.id << "|" << n.printMode << "\n";
        }
        // Coklu degisken soketlerini kaydet
        if (!n.multiAssignments.empty()) {
            out << "MULTI_ASSIGN|" << n.id;
            for (const auto& a : n.multiAssignments) {
                out << "|" << a.varName << "|" << (int)a.varType << "|" << a.valueExpr;
            }
            out << "\n";
        }
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

            // BODY_STMTS satiri — Normal dugum satirina dokunma
            if (!tokens.empty() && tokens[0] == "BODY_STMTS") {
                if (tokens.size() >= 3) {
                    int targetId = std::stoi(tokens[1]);
                    FlowNode* bn = GetNode(targetId);
                    if (bn) {
                        bn->loopBodyStmts.clear();
                        for (size_t ti = 2; ti < tokens.size(); ti++)
                            bn->loopBodyStmts.push_back(tokens[ti]);
                    }
                }
                continue; // Bu satiri dugum olarak isleme
            }

            if (!tokens.empty() && tokens[0] == "MATH_OP") {
                if (tokens.size() >= 6) {
                    int targetId = std::stoi(tokens[1]);
                    FlowNode* mn = GetNode(targetId);
                    if (mn) {
                        mn->mathTarget = tokens[2];
                        mn->mathLeft = tokens[3];
                        mn->mathOp = tokens[4];
                        mn->mathRight = tokens[5];
                    }
                }
                continue;
            }

            if (!tokens.empty() && tokens[0] == "PRINT_MODE") {
                if (tokens.size() >= 3) {
                    int targetId = std::stoi(tokens[1]);
                    FlowNode* pn = GetNode(targetId);
                    if (pn) {
                        pn->printMode = std::stoi(tokens[2]);
                    }
                }
                continue;
            }

            if (!tokens.empty() && tokens[0] == "MULTI_ASSIGN") {
                if (tokens.size() >= 5) {
                    int targetId = std::stoi(tokens[1]);
                    FlowNode* vn = GetNode(targetId);
                    if (vn) {
                        vn->multiAssignments.clear();
                        for (size_t ai = 2; ai + 2 < tokens.size(); ai += 3) {
                            vn->multiAssignments.push_back({ tokens[ai], (TypeKind)std::stoi(tokens[ai + 1]), tokens[ai + 2] });
                        }
                    }
                }
                continue;
            }

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
    ss << "#include <vector>\n";
    ss << "#include <list>\n";
    ss << "#include <stack>\n";
    ss << "#include <queue>\n";
    ss << "#include <array>\n";
    ss << "#include <memory>\n";
    ss << "#include <cmath>\n";
    ss << "#include <windows.h>\n\n";

    // ── KULLANICI TANIMLI VERI YAPILARI (STRUCTS) ─────────────
    bool hasStructs = false;
    for (const auto& n : nodes) {
        if (n.kind == NodeKind::StructDef) {
            if (!hasStructs) {
                ss << "// ── KULLANICI TANIMLI VERI YAPILARI (STRUCTS) ─────────────\n";
                hasStructs = true;
            }
            ss << "struct " << n.structName << " {\n";
            for (const auto& m : n.structMembers) {
                if (m.hasDefault && !m.defaultValue.empty()) {
                    ss << "    " << TypeKindToString(m.type) << " " << m.name << " = " << m.defaultValue << ";\n";
                } else {
                    ss << "    " << TypeKindToString(m.type) << " " << m.name << ";\n";
                }
            }
            if (n.hasDestructor) {
                ss << "\n    // RAII Yikici Metot (Destructor)\n";
                ss << "    ~" << n.structName << "() {\n";
                ss << "        std::cout << \"[DESTRUCTOR ~" << n.structName << "] " << n.destructorMsg << "\" << std::endl;\n";
                ss << "    }\n";
            }
            ss << "};\n\n";
        }
    }

    ss << "int main() {\n";
    ss << "    // Windows Konsolunu UTF-8 Moduna Al (Turkce Karakter Destegi)\n";
    ss << "    SetConsoleOutputCP(65001);\n";
    ss << "    SetConsoleCP(65001);\n\n";
    ss << "    std::cout << \"=== " << projectName << " CALISIYOR ===\" << std::endl;\n\n";

    // Helper lambda: verilen pinden çıkan düğümü bul (Akıllı Çift Yönlü Arama)
    auto getTargetNode = [&](int fromNodeId, int pinIndex) -> int {
        for (const auto& w : wires) {
            // 1. Normal yon: Bu dugumden cikan kablo
            if (w.fromNode == fromNodeId && w.fromPin == pinIndex) {
                return w.toNode;
            }
            // 2. Ters yon: Kullanici kabloyu diger dugumden bu dugume dogru bagladiysa
            if (w.toNode == fromNodeId && w.toPin == pinIndex) {
                return w.fromNode;
            }
        }
        return -1;
    };

    // Recursive / Stack-based zincir kod üretici fonksiyon
    std::set<int> visited;
    std::set<std::string> declaredDataStructures;
    std::function<void(int, int)> generateChain = [&](int nodeId, int indent) {
        int curr = nodeId;
        while (curr > 0) {
            if (visited.count(curr)) break;
            visited.insert(curr);

            FlowNode* n = GetNode(curr);
            if (!n) break;

            std::string tabs(indent * 4, ' ');

            if (n->kind == NodeKind::StructDef) {
                ss << tabs << "// Struct Bildirimi: " << n->structName << "\n";
                ss << tabs << "std::cout << \"[STRUCT] " << n->structName << " veri yapisi aktif.\" << std::endl;\n\n";
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::VariableDeclare) {
                std::string constPrefix = n->isConst ? "const " : "";
                std::string resolvedType = n->isCustomType ? n->customTypeName : TypeKindToString(n->varType);

                if (n->pointerKind == PointerKind::SharedPtr) {
                    ss << tabs << "// Akilli Isaretci (std::shared_ptr) ile Heap Tahsisi\n";
                    if (n->isCustomType) {
                        ss << tabs << constPrefix << "std::shared_ptr<" << resolvedType << "> " << n->targetVar << " = std::make_shared<" << resolvedType << ">();\n";
                    } else {
                        ss << tabs << constPrefix << "std::shared_ptr<" << resolvedType << "> " << n->targetVar << " = std::make_shared<" << resolvedType << ">(" << n->expr << ");\n";
                    }
                    ss << tabs << "std::cout << \"[SMART PTR] " << n->targetVar << " olusturuldu (Ref Count: \" << " << n->targetVar << ".use_count() << \")\" << std::endl;\n";
                    if (!n->isCustomType) {
                        ss << tabs << "std::cout << \"[SMART PTR] *" << n->targetVar << " = \" << *" << n->targetVar << " << std::endl;\n";
                    }
                    ss << "\n";
                }
                else if (n->pointerKind == PointerKind::UniquePtr) {
                    ss << tabs << "// Tekil Isaretci (std::unique_ptr) ile Heap Tahsisi\n";
                    if (n->isCustomType) {
                        ss << tabs << constPrefix << "std::unique_ptr<" << resolvedType << "> " << n->targetVar << " = std::make_unique<" << resolvedType << ">();\n";
                    } else {
                        ss << tabs << constPrefix << "std::unique_ptr<" << resolvedType << "> " << n->targetVar << " = std::make_unique<" << resolvedType << ">(" << n->expr << ");\n";
                    }
                    ss << tabs << "std::cout << \"[UNIQUE PTR] " << n->targetVar << " heap uzerinde tekil olarak tahsis edildi.\" << std::endl;\n\n";
                }
                else {
                    if (n->isCustomType) {
                        ss << tabs << "// Nesne Tanimlama (" << resolvedType << ")\n";
                        ss << tabs << constPrefix << resolvedType << " " << n->targetVar << ";\n";
                        ss << tabs << "std::cout << \"[STRUCT INSTANCE] " << n->targetVar << " stack uzerinde olusturuldu.\" << std::endl;\n\n";
                    } else {
                        ss << tabs << "// Degisken Tanimlama (" << (n->isConst ? "const " : "") << n->targetVar << ")\n";
                        std::string printCast = (n->varType == TypeKind::UInt8) ? "+(int)" : "";
                        if (!n->expr.empty()) {
                            ss << tabs << constPrefix << resolvedType << " " << n->targetVar << " = " << n->expr << ";\n";
                            ss << tabs << "std::cout << \"[LOG] " << n->targetVar << " = \" << " << printCast << n->targetVar << " << std::endl;\n\n";
                        } else {
                            ss << tabs << constPrefix << resolvedType << " " << n->targetVar << ";\n\n";
                        }
                        for (const auto& extra : n->multiAssignments) {
                            if (!extra.valueExpr.empty()) {
                                ss << tabs << constPrefix << TypeKindToString(extra.varType) << " " << extra.varName << " = " << extra.valueExpr << ";\n";
                            } else {
                                ss << tabs << constPrefix << TypeKindToString(extra.varType) << " " << extra.varName << ";\n";
                            }
                        }
                    }

                }
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::VectorOp) {
                std::string targetDS = !n->dsName.empty() ? n->dsName : n->vectorName;
                std::string op = !n->dsOpType.empty() ? n->dsOpType : n->vectorOpType;
                std::string arg = !n->dsOpArg.empty() ? n->dsOpArg : n->vectorOpArg;

                if (!declaredDataStructures.count(targetDS)) {
                    ss << tabs << "// Veri Yapisi Tanimlama (Dinamik Dizi)\n";
                    ss << tabs << "std::vector<int> " << targetDS << ";\n";
                    declaredDataStructures.insert(targetDS);
                }

                ss << tabs << "// std::vector Islemi: " << targetDS << "." << op << "()\n";
                if (op == "push_back") {
                    ss << tabs << targetDS << ".push_back(" << arg << ");\n";
                    ss << tabs << "std::cout << \"[VECTOR] " << targetDS << ".push_back(" << arg << ") -> Yeni Boyut: \" << " << targetDS << ".size() << std::endl;\n\n";
                } else if (op == "pop_back") {
                    ss << tabs << "if (!" << targetDS << ".empty()) " << targetDS << ".pop_back();\n";
                    ss << tabs << "std::cout << \"[VECTOR] " << targetDS << ".pop_back() -> Yeni Boyut: \" << " << targetDS << ".size() << std::endl;\n\n";
                } else if (op == "clear") {
                    ss << tabs << targetDS << ".clear();\n";
                    ss << tabs << "std::cout << \"[VECTOR] " << targetDS << ".clear() calistirildi.\" << std::endl;\n\n";
                }
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::ListOp) {
                if (!declaredDataStructures.count(n->dsName)) {
                    ss << tabs << "// Veri Yapisi Tanimlama (Bagli Liste)\n";
                    ss << tabs << "std::list<int> " << n->dsName << ";\n";
                    declaredDataStructures.insert(n->dsName);
                }

                ss << tabs << "// std::list Islemi (Bagli Liste): " << n->dsName << "." << n->dsOpType << "()\n";
                if (n->dsOpType == "push_back") {
                    ss << tabs << n->dsName << ".push_back(" << n->dsOpArg << ");\n";
                    ss << tabs << "std::cout << \"[LIST] " << n->dsName << ".push_back(" << n->dsOpArg << ") -> Boyut: \" << " << n->dsName << ".size() << std::endl;\n\n";
                } else if (n->dsOpType == "push_front") {
                    ss << tabs << n->dsName << ".push_front(" << n->dsOpArg << ");\n";
                    ss << tabs << "std::cout << \"[LIST] " << n->dsName << ".push_front(" << n->dsOpArg << ") -> Boyut: \" << " << n->dsName << ".size() << std::endl;\n\n";
                } else if (n->dsOpType == "pop_front") {
                    ss << tabs << "if (!" << n->dsName << ".empty()) " << n->dsName << ".pop_front();\n";
                    ss << tabs << "std::cout << \"[LIST] " << n->dsName << ".pop_front() (Bastan Silindi - O(1))\" << std::endl;\n\n";
                } else if (n->dsOpType == "pop_back") {
                    ss << tabs << "if (!" << n->dsName << ".empty()) " << n->dsName << ".pop_back();\n";
                    ss << tabs << "std::cout << \"[LIST] " << n->dsName << ".pop_back() (Sondan Silindi)\" << std::endl;\n\n";
                } else if (n->dsOpType == "clear") {
                    ss << tabs << n->dsName << ".clear();\n";
                    ss << tabs << "std::cout << \"[LIST] " << n->dsName << ".clear() tamamlandi.\" << std::endl;\n\n";
                }
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::StackOp) {
                if (!declaredDataStructures.count(n->dsName)) {
                    ss << tabs << "// Veri Yapisi Tanimlama (Yigin / LIFO)\n";
                    ss << tabs << "std::stack<int> " << n->dsName << ";\n";
                    declaredDataStructures.insert(n->dsName);
                }

                ss << tabs << "// std::stack Islemi (Yigin / LIFO): " << n->dsName << "." << n->dsOpType << "()\n";
                if (n->dsOpType == "push") {
                    ss << tabs << n->dsName << ".push(" << n->dsOpArg << ");\n";
                    ss << tabs << "std::cout << \"[STACK] " << n->dsName << ".push(" << n->dsOpArg << ") -> Tepe Elemani (top): \" << " << n->dsName << ".top() << std::endl;\n\n";
                } else if (n->dsOpType == "pop") {
                    ss << tabs << "if (!" << n->dsName << ".empty()) { std::cout << \"[STACK] Pop Edilen Eleman: \" << " << n->dsName << ".top() << std::endl; " << n->dsName << ".pop(); }\n\n";
                } else if (n->dsOpType == "top") {
                    ss << tabs << "if (!" << n->dsName << ".empty()) std::cout << \"[STACK TOP] Tepe Eleman: \" << " << n->dsName << ".top() << std::endl;\n\n";
                }
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::QueueOp) {
                if (!declaredDataStructures.count(n->dsName)) {
                    ss << tabs << "// Veri Yapisi Tanimlama (Kuyruk / FIFO)\n";
                    ss << tabs << "std::queue<int> " << n->dsName << ";\n";
                    declaredDataStructures.insert(n->dsName);
                }

                ss << tabs << "// std::queue Islemi (Kuyruk / FIFO): " << n->dsName << "." << n->dsOpType << "()\n";
                if (n->dsOpType == "push") {
                    ss << tabs << n->dsName << ".push(" << n->dsOpArg << ");\n";
                    ss << tabs << "std::cout << \"[QUEUE] " << n->dsName << ".push(" << n->dsOpArg << ") -> Kuyruk Basi (front): \" << " << n->dsName << ".front() << std::endl;\n\n";
                } else if (n->dsOpType == "pop") {
                    ss << tabs << "if (!" << n->dsName << ".empty()) { std::cout << \"[QUEUE] Pop Edilen (Cikan) Eleman: \" << " << n->dsName << ".front() << std::endl; " << n->dsName << ".pop(); }\n\n";
                } else if (n->dsOpType == "front") {
                    ss << tabs << "if (!" << n->dsName << ".empty()) std::cout << \"[QUEUE FRONT] Ilk Eleman: \" << " << n->dsName << ".front() << std::endl;\n\n";
                }
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::ArrayOp) {
                if (!declaredDataStructures.count(n->dsName)) {
                    ss << tabs << "// Sabit Boyutlu Dizi Tanimlama\n";
                    ss << tabs << "int " << n->dsName << "[" << n->arraySize << "] = {0};\n";
                    declaredDataStructures.insert(n->dsName);
                }

                ss << tabs << "// Sabit Boyutlu Dizi Islemi (Array Indexing): " << n->dsName << "[" << n->arrayIndex << "] = " << n->dsOpArg << "\n";
                ss << tabs << n->dsName << "[" << n->arrayIndex << "] = " << n->dsOpArg << ";\n";
                ss << tabs << "std::cout << \"[ARRAY] " << n->dsName << "[" << n->arrayIndex << "] = \" << " << n->dsName << "[" << n->arrayIndex << "] << std::endl;\n\n";
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::Assign) {
                ss << tabs << "// Deger Atama\n";
                ss << tabs << n->targetVar << " = " << n->expr << ";\n";
                for (const auto& extra : n->multiAssignments) {
                    ss << tabs << extra.varName << " = " << extra.valueExpr << ";\n";
                }
                ss << "\n";
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::Print) {
                ss << tabs << "// Ekrana Cikti Basma\n";
                if (n->printMode == 1) {
                    // Sadece Degisken Degeri
                    std::string varName = !n->targetVar.empty() ? n->targetVar : "x";
                    ss << tabs << "std::cout << " << varName << " << std::endl;\n\n";
                } else if (n->printMode == 2) {
                    // Metin + Degisken
                    std::string varName = !n->targetVar.empty() ? n->targetVar : "x";
                    ss << tabs << "std::cout << \"" << n->ioText << " \" << " << varName << " << std::endl;\n\n";
                } else {
                    // Duz Metin (Varsayilan)
                    ss << tabs << "std::cout << \"" << n->ioText << "\" << std::endl;\n\n";
                }
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::Input) {
                std::string inVar = !n->targetVar.empty() ? n->targetVar : "inputVal";
                ss << tabs << "// Kullanicidan Girdi Alma (std::cin >> " << inVar << ")\n";
                ss << tabs << "std::cin >> " << inVar << ";\n\n";
                curr = getTargetNode(curr, 1);
            }
            else if (n->kind == NodeKind::MathOp) {
                std::string target = !n->mathTarget.empty() ? n->mathTarget : "sonuc";
                std::string left = !n->mathLeft.empty() ? n->mathLeft : "0";
                std::string right = !n->mathRight.empty() ? n->mathRight : "0";
                std::string op = !n->mathOp.empty() ? n->mathOp : "+";

                ss << tabs << "// Matematiksel Islem: " << target << "\n";
                if (op == "sqrt") {
                    ss << tabs << target << " = std::sqrt(" << left << ");\n";
                    ss << tabs << "std::cout << \"[ISLEM] " << target << " = sqrt(\" << +" << left << " << \") -> \" << +" << target << " << std::endl;\n\n";
                } else if (op == "pow") {
                    ss << tabs << target << " = std::pow(" << left << ", " << right << ");\n";
                    ss << tabs << "std::cout << \"[ISLEM] " << target << " = pow(\" << +" << left << " << \", \" << +" << right << " << \") -> \" << +" << target << " << std::endl;\n\n";
                } else if (op == "fact") {
                    ss << tabs << target << " = 1;\n";
                    ss << tabs << "for (int i = 1; i <= " << left << "; ++i) { " << target << " *= i; }\n";
                    ss << tabs << "std::cout << \"[ISLEM] " << target << " = \" << +" << left << " << \"! (Faktoriyel) -> \" << +" << target << " << std::endl;\n\n";
                } else {
                    // +, -, *, /, %
                    ss << tabs << target << " = " << left << " " << op << " " << right << ";\n";
                    ss << tabs << "std::cout << \"[ISLEM] " << target << " = \" << +" << left << " << \" " << op << " \" << +" << right << " << \" -> \" << +" << target << " << std::endl;\n\n";
                }
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
                // Inspector'dan eklenen gövde artış/atama satırları
                for (const auto& stmt : n->loopBodyStmts) {
                    if (!stmt.empty())
                        ss << tabs << "    " << stmt << ";\n";
                }
                ss << tabs << "}\n\n";
                curr = getTargetNode(curr, 2); // Dongu bitince completed pininden devam
            }
            else if (n->kind == NodeKind::LoopDoWhile) {
                ss << tabs << "// Do-While Dongusu\n";
                ss << tabs << "do {\n";
                int bodyNode = getTargetNode(curr, 1);
                if (bodyNode > 0) generateChain(bodyNode, indent + 1);
                // Inspector'dan eklenen gövde artış/atama satırları
                for (const auto& stmt : n->loopBodyStmts) {
                    if (!stmt.empty())
                        ss << tabs << "    " << stmt << ";\n";
                }
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
            else if (n->kind == NodeKind::SwitchCase) {
                ss << tabs << "// Switch-Case Coklu Karar Yapisi\n";
                ss << tabs << "switch (" << n->switchVar << ") {\n";
                
                // Case 1
                ss << tabs << "    case " << (n->caseValues.size() > 0 ? n->caseValues[0] : "1") << ": {\n";
                int case1Node = getTargetNode(curr, 1);
                if (case1Node > 0) generateChain(case1Node, indent + 2);
                ss << tabs << "        break;\n";
                ss << tabs << "    }\n";

                // Case 2
                ss << tabs << "    case " << (n->caseValues.size() > 1 ? n->caseValues[1] : "2") << ": {\n";
                int case2Node = getTargetNode(curr, 2);
                if (case2Node > 0) generateChain(case2Node, indent + 2);
                ss << tabs << "        break;\n";
                ss << tabs << "    }\n";

                // Default
                ss << tabs << "    default: {\n";
                int defNode = getTargetNode(curr, 3);
                if (defNode > 0) generateChain(defNode, indent + 2);
                ss << tabs << "        break;\n";
                ss << tabs << "    }\n";

                ss << tabs << "}\n\n";
                curr = getTargetNode(curr, 4); // Switch bitince Exit pininden devam
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
