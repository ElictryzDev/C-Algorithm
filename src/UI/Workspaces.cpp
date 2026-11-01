#include "Workspaces.hpp"
#include <windows.h>
#include <shellapi.h>
#include <cmath>
#include <sstream>
#include <cstdio>

// ── 1. Proje Karsilama Ekrani (Project Gating) ───────────────────────────
void RenderProjectGatingModal(ProjectManager& pm) {
    bool isEn = (pm.currentLanguage == AppLanguage::English);
    bool isLight = (pm.currentTheme == AppTheme::LightPastelNeon);
    ImVec4 headingCol = isLight ? ImVec4(0.05f, 0.58f, 0.53f, 1.0f) : ImVec4(0.0f, 0.85f, 1.0f, 1.0f);
    ImVec4 subCol = isLight ? ImVec4(0.35f, 0.42f, 0.50f, 1.0f) : ImVec4(0.6f, 0.65f, 0.75f, 1.0f);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(620, 480));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    ImGui::Begin(isEn ? "C-ALGORITHM — PROJECT GATEWAY" : "C-ALGORITHM — PROJE GİRİŞ PANELİ", nullptr, flags);

    ImGui::TextColored(headingCol, isEn ? "WELCOME TO C-ALGORITHM NATIVE IDE" : "C-ALGORITHM NATIVE IDE'YE HOŞ GELDİNİZ");
    ImGui::TextColored(subCol, isEn ? "Select an existing project or create a new one to continue." : "Devam etmek için lütfen bir proje seçin veya yeni bir proje oluşturun.");
    ImGui::Separator();
    ImGui::Spacing();

    static char newProjName[64] = "Algoritma_Test_01";
    static char openFilePath[256] = "";

    // Sekmeler: Yeni Proje / Var Olani Ac
    if (ImGui::BeginTabBar("GatingTabs")) {
        if (ImGui::BeginTabItem(isEn ? "Create New Project" : "Yeni Proje Oluştur")) {
            ImGui::Spacing();
            ImGui::Text(isEn ? "Project Name:" : "Proje Adı:");
            ImGui::InputText("##pname", newProjName, sizeof(newProjName));

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.5f, 0.7f, 0.9f, 1.0f), isEn ? "Save Path: Desktop/C-Algorithm/projects/%s.calg" : "Kayıt Konumu: Desktop/C-Algorithm/projects/%s.calg", newProjName);
            ImGui::Spacing();

            if (ImGui::Button(isEn ? "Create Project & Start" : "Projeyi Oluştur ve Başla", ImVec2(240, 42))) {
                pm.CreateNewProject(newProjName, "C:/Users/ASUS TUF GAMING/Desktop/C-Algorithm/projects");
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(isEn ? "List & Open Saved Projects" : "Kayıtlı Projeleri Listele ve Aç")) {
            ImGui::Spacing();
            std::vector<std::string> savedFiles = pm.GetSavedProjectFiles("C:/Users/ASUS TUF GAMING/Desktop/C-Algorithm/projects");

            ImGui::Text(isEn ? "Found Projects (%d total):" : "Bulunan Projeler (%d adet):", (int)savedFiles.size());

            if (ImGui::BeginListBox("##savedProjList", ImVec2(-1, 160))) {
                for (size_t i = 0; i < savedFiles.size(); i++) {
                    const std::string& full = savedFiles[i];
                    size_t slash = full.find_last_of("/\\");
                    std::string displayName = (slash != std::string::npos) ? full.substr(slash + 1) : full;

                    bool isSelected = (std::string(openFilePath) == full);
                    if (ImGui::Selectable(displayName.c_str(), isSelected)) {
                        strncpy(openFilePath, full.c_str(), sizeof(openFilePath));
                    }
                }
                ImGui::EndListBox();
            }

            ImGui::Spacing();
            ImGui::Text(isEn ? "Selected File:" : "Seçilen Dosya:");
            ImGui::InputText("##selectedPath", openFilePath, sizeof(openFilePath));

            ImGui::Spacing();
            if (ImGui::Button("📂 Secili Projeyi Yukle", ImVec2(220, 38))) {
                if (openFilePath[0] != '\0') {
                    pm.OpenProject(openFilePath);
                } else {
                    pm.statusMessage = "[UYARI] Lutfen listeden bir proje secin!";
                }
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 42);
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "%s", pm.statusMessage.c_str());

    ImGui::End();
}

// ── Kamera Pan & Zoom Global Durumu ─────────────────────────────────────
static ImVec2 g_flowPanOffset = { 0.0f, 0.0f };
static float g_flowZoomLevel = 1.0f;

// =========================================================================
// C-ALGORITHM MODERN FLOWCHART & NEON ENGINE HELPERS
// =========================================================================

enum class FlowchartShape {
    Pill,           // Start, End (Kapsül)
    Diamond,        // BranchIf (Baklava Dilimi)
    Parallelogram,  // Input, Print (Eğimli Paralelkenar)
    Hexagon,        // LoopWhile, LoopDoWhile, LoopFor (Altıgen)
    Rectangle       // VariableDeclare, Assign, MathOp, StructDef, DataStructures (Modern Kart)
};

inline FlowchartShape GetFlowchartShape(NodeKind kind) {
    switch (kind) {
        case NodeKind::Start:
        case NodeKind::End:
        case NodeKind::LoopEnd:
            return FlowchartShape::Pill;
        case NodeKind::BranchIf:
            return FlowchartShape::Diamond;
        case NodeKind::Input:
        case NodeKind::Print:
            return FlowchartShape::Parallelogram;
        case NodeKind::LoopWhile:
        case NodeKind::LoopDoWhile:
        case NodeKind::LoopFor:
            return FlowchartShape::Hexagon;
        default:
            return FlowchartShape::Rectangle;
    }
}

// Düğüm Geometrisi ve Pin Koordinatlarını Flowchart Standardına Göre Eşitle
inline void SyncNodeGeometryAndPins(FlowNode& node) {
    switch (node.kind) {
        case NodeKind::Start:
            node.width = 190.0f; node.height = 65.0f;
            if (!node.pins.empty()) {
                node.pins[0].relX = node.width - 14.0f;
                node.pins[0].relY = node.height * 0.5f;
            }
            break;
        case NodeKind::End:
            node.width = 190.0f; node.height = 65.0f;
            if (!node.pins.empty()) {
                node.pins[0].relX = 14.0f;
                node.pins[0].relY = node.height * 0.5f;
            }
            break;
        case NodeKind::LoopEnd:
            node.width = 190.0f; node.height = 65.0f;
            if (node.pins.size() >= 2) {
                node.pins[0].relX = 14.0f;           node.pins[0].relY = node.height * 0.5f;
                node.pins[1].relX = node.width - 14.0f; node.pins[1].relY = node.height * 0.5f;
            }
            break;
        case NodeKind::BranchIf:
            node.width = 240.0f; node.height = 110.0f;
            if (node.pins.size() >= 3) {
                node.pins[0].relX = 14.0f;           node.pins[0].relY = node.height * 0.5f;  // Giriş (Sol)
                node.pins[1].relX = node.width - 14.0f; node.pins[1].relY = node.height * 0.5f;  // True / Evet (Sağ)
                node.pins[2].relX = node.width * 0.5f;  node.pins[2].relY = node.height - 12.0f; // False / Hayır (Alt)
            }
            break;
        case NodeKind::Input:
            node.width = 230.0f; node.height = 80.0f;
            if (node.pins.size() >= 2) {
                node.pins[0].relX = 14.0f;           node.pins[0].relY = node.height * 0.5f;
                node.pins[1].relX = node.width - 14.0f; node.pins[1].relY = node.height * 0.5f;
            }
            break;
        case NodeKind::Print:
            node.width = 240.0f; node.height = 85.0f;
            if (node.pins.size() >= 2) {
                node.pins[0].relX = 14.0f;           node.pins[0].relY = node.height * 0.5f;
                node.pins[1].relX = node.width - 14.0f; node.pins[1].relY = node.height * 0.5f;
            }
            break;
        case NodeKind::LoopWhile:
        case NodeKind::LoopDoWhile:
        case NodeKind::LoopFor:
            node.width = 240.0f; node.height = 100.0f;
            if (node.pins.size() >= 2) {
                node.pins[0].relX = 14.0f;           node.pins[0].relY = node.height * 0.5f;
                node.pins[1].relX = node.width - 14.0f; node.pins[1].relY = node.height * 0.5f;
            }
            break;
        case NodeKind::VariableDeclare:
        case NodeKind::Assign:
        case NodeKind::MathOp:
            node.width = 230.0f; node.height = 90.0f;
            if (node.pins.size() >= 2) {
                node.pins[0].relX = 12.0f;           node.pins[0].relY = node.height * 0.5f;
                node.pins[1].relX = node.width - 12.0f; node.pins[1].relY = node.height * 0.5f;
            }
            break;
        default:
            if (node.pins.size() >= 2) {
                node.pins[0].relX = 12.0f;           node.pins[0].relY = node.height * 0.5f;
                node.pins[1].relX = node.width - 12.0f; node.pins[1].relY = node.height * 0.5f;
            }
            break;
    }
}

// Çok Katmanlı Neon Işıma Dış Çizgisi (Polyline)
inline void DrawNeonPolyline(ImDrawList* drawList, const ImVec2* points, int count, ImU32 baseCol, float baseThick, bool closed = true) {
    ImVec4 c = ImGui::ColorConvertU32ToFloat4(baseCol);
    ImU32 outerCol = ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, c.w * 0.20f));
    ImU32 midCol   = ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, c.w * 0.50f));

    drawList->AddPolyline(points, count, outerCol, closed ? ImDrawFlags_Closed : ImDrawFlags_None, baseThick + 6.0f);
    drawList->AddPolyline(points, count, midCol,   closed ? ImDrawFlags_Closed : ImDrawFlags_None, baseThick + 2.5f);
    drawList->AddPolyline(points, count, baseCol,  closed ? ImDrawFlags_Closed : ImDrawFlags_None, baseThick);
}

// Çok Katmanlı Neon Işıma Dikdörtgeni (Rect)
inline void DrawNeonRect(ImDrawList* drawList, ImVec2 pMin, ImVec2 pMax, ImU32 baseCol, float rounding, float baseThick) {
    ImVec4 c = ImGui::ColorConvertU32ToFloat4(baseCol);
    ImU32 outerCol = ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, c.w * 0.20f));
    ImU32 midCol   = ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, c.w * 0.50f));

    drawList->AddRect(pMin, pMax, outerCol, rounding, 0, baseThick + 6.0f);
    drawList->AddRect(pMin, pMax, midCol,   rounding, 0, baseThick + 2.5f);
    drawList->AddRect(pMin, pMax, baseCol,  rounding, 0, baseThick);
}

// Ok Başı Çizim Yardımcısı (Bézier Telinin Hedef Bloğa Saplanan Ucu)
inline void DrawWireArrowhead(ImDrawList* drawList, ImVec2 tip, ImVec2 dir, float size, ImU32 col) {
    float len = hypotf(dir.x, dir.y);
    if (len < 0.001f) { dir = ImVec2(1.0f, 0.0f); len = 1.0f; }
    ImVec2 u(dir.x / len, dir.y / len);
    ImVec2 v(-u.y, u.x);

    ImVec2 p0 = tip;
    ImVec2 p1(tip.x - u.x * size + v.x * (size * 0.55f),
              tip.y - u.y * size + v.y * (size * 0.55f));
    ImVec2 p2(tip.x - u.x * size - v.x * (size * 0.55f),
              tip.y - u.y * size - v.y * (size * 0.55f));

    drawList->AddTriangleFilled(p0, p1, p2, col);
}

struct NodeColorTheme {
    ImU32 bgCol;
    ImU32 neonCol;
    ImU32 headCol;
    ImU32 textCol;
    const char* categoryBadge;
};

inline NodeColorTheme GetNodeTheme(NodeKind kind, AppTheme theme, bool isEn) {
    bool isLight = (theme == AppTheme::LightPastelNeon);

    if (isLight) {
        // Açık Pastel Pembe & Petrol Yeşili Lüks Tema
        ImU32 txtCol = IM_COL32(15, 23, 42, 255); // Derin net siyah/arduvaz
        switch (kind) {
            case NodeKind::Start:
                return { IM_COL32(240, 253, 248, 252), IM_COL32(13, 148, 136, 255), IM_COL32(13, 148, 136, 40), txtCol, isEn ? "Program Start" : "Yazılımın Başlangıcı" };
            case NodeKind::End:
                return { IM_COL32(255, 241, 242, 252), IM_COL32(244, 63, 94, 255), IM_COL32(244, 63, 94, 40), txtCol, isEn ? "Program End" : "Yazılımın Sonlanışı" };
            case NodeKind::VariableDeclare:
                return { IM_COL32(240, 249, 255, 252), IM_COL32(2, 132, 199, 255), IM_COL32(2, 132, 199, 40), txtCol, isEn ? "Variable Declare" : "Değişken Tanımlama" };
            case NodeKind::Assign:
                return { IM_COL32(250, 245, 255, 252), IM_COL32(147, 51, 234, 255), IM_COL32(147, 51, 234, 40), txtCol, isEn ? "Assignment" : "Değer Atama" };
            case NodeKind::MathOp:
                return { IM_COL32(240, 253, 250, 252), IM_COL32(13, 148, 136, 255), IM_COL32(13, 148, 136, 40), txtCol, isEn ? "Math Operation" : "Matematiksel İşlem" };
            case NodeKind::Input:
                return { IM_COL32(236, 253, 245, 252), IM_COL32(16, 185, 129, 255), IM_COL32(16, 185, 129, 40), txtCol, isEn ? "Input Read (cin)" : "Kullanıcı Girdisi (cin)" };
            case NodeKind::Print:
                return { IM_COL32(245, 243, 255, 252), IM_COL32(124, 58, 237, 255), IM_COL32(124, 58, 237, 40), txtCol, isEn ? "Print Output (cout)" : "Konsol Çıktısı (cout)" };
            case NodeKind::BranchIf:
                return { IM_COL32(254, 252, 232, 252), IM_COL32(245, 158, 11, 255), IM_COL32(245, 158, 11, 45), txtCol, isEn ? "Condition (if)" : "Koşul / Karar (if)" };
            case NodeKind::LoopWhile:
                return { IM_COL32(253, 242, 248, 252), IM_COL32(244, 63, 94, 255), IM_COL32(244, 63, 94, 40), txtCol, isEn ? "While Loop" : "While Döngüsü" };
            case NodeKind::LoopDoWhile:
                return { IM_COL32(253, 242, 248, 252), IM_COL32(244, 63, 94, 255), IM_COL32(244, 63, 94, 40), txtCol, isEn ? "Do-While Loop" : "Do-While Döngüsü" };
            case NodeKind::LoopFor:
                return { IM_COL32(253, 242, 248, 252), IM_COL32(244, 63, 94, 255), IM_COL32(244, 63, 94, 40), txtCol, isEn ? "For Loop" : "For Döngüsü" };
            case NodeKind::LoopEnd:
                return { IM_COL32(254, 243, 199, 252), IM_COL32(217, 119, 6, 255), IM_COL32(217, 119, 6, 40), txtCol, isEn ? "Loop End" : "Döngü Sonu" };
            case NodeKind::SwitchCase:
                return { IM_COL32(254, 243, 199, 252), IM_COL32(217, 119, 6, 255), IM_COL32(217, 119, 6, 40), txtCol, isEn ? "Switch-Case" : "Çoklu Seçim (switch)" };
            case NodeKind::StructDef:
                return { IM_COL32(254, 243, 199, 252), IM_COL32(217, 119, 6, 255), IM_COL32(217, 119, 6, 40), txtCol, isEn ? "Struct Definition" : "Özel Yapı (struct)" };
            default:
                return { IM_COL32(238, 242, 255, 252), IM_COL32(79, 70, 229, 255), IM_COL32(79, 70, 229, 40), txtCol, isEn ? "Data Structure" : "Veri Yapısı (Koleksiyon)" };
        }
    } else {
        // Koyu Cyberpunk Neon
        ImU32 txtCol = IM_COL32(245, 250, 255, 255);
        switch (kind) {
            case NodeKind::Start:
                return { IM_COL32(10, 32, 22, 240), IM_COL32(16, 185, 129, 255), IM_COL32(16, 185, 129, 45), txtCol, isEn ? "Program Start" : "Yazılımın Başlangıcı" };
            case NodeKind::End:
                return { IM_COL32(36, 12, 16, 240), IM_COL32(239, 68, 68, 255), IM_COL32(239, 68, 68, 45), txtCol, isEn ? "Program End" : "Yazılımın Sonlanışı" };
            case NodeKind::VariableDeclare:
                return { IM_COL32(12, 26, 44, 240), IM_COL32(14, 165, 233, 255), IM_COL32(14, 165, 233, 50), txtCol, isEn ? "Variable Declare" : "Değişken Tanımlama" };
            case NodeKind::Assign:
                return { IM_COL32(26, 16, 42, 240), IM_COL32(168, 85, 247, 255), IM_COL32(168, 85, 247, 50), txtCol, isEn ? "Assignment" : "Değer Atama" };
            case NodeKind::MathOp:
                return { IM_COL32(10, 32, 38, 240), IM_COL32(6, 182, 212, 255), IM_COL32(6, 182, 212, 50), txtCol, isEn ? "Math Operation" : "Matematiksel İşlem" };
            case NodeKind::Input:
                return { IM_COL32(10, 34, 34, 240), IM_COL32(20, 184, 166, 255), IM_COL32(20, 184, 166, 50), txtCol, isEn ? "Input Read (cin)" : "Kullanıcı Girdisi (cin)" };
            case NodeKind::Print:
                return { IM_COL32(24, 18, 46, 240), IM_COL32(139, 92, 246, 255), IM_COL32(139, 92, 246, 50), txtCol, isEn ? "Print Output (cout)" : "Konsol Çıktısı (cout)" };
            case NodeKind::BranchIf:
                return { IM_COL32(38, 26, 10, 240), IM_COL32(245, 158, 11, 255), IM_COL32(245, 158, 11, 50), txtCol, isEn ? "Condition (if)" : "Koşul / Karar (if)" };
            case NodeKind::LoopWhile:
                return { IM_COL32(38, 14, 24, 240), IM_COL32(244, 63, 94, 255), IM_COL32(244, 63, 94, 50), txtCol, isEn ? "While Loop" : "While Döngüsü" };
            case NodeKind::LoopDoWhile:
                return { IM_COL32(38, 14, 24, 240), IM_COL32(244, 63, 94, 255), IM_COL32(244, 63, 94, 50), txtCol, isEn ? "Do-While Loop" : "Do-While Döngüsü" };
            case NodeKind::LoopFor:
                return { IM_COL32(38, 14, 24, 240), IM_COL32(244, 63, 94, 255), IM_COL32(244, 63, 94, 50), txtCol, isEn ? "For Loop" : "For Döngüsü" };
            case NodeKind::LoopEnd:
                return { IM_COL32(38, 26, 10, 240), IM_COL32(245, 158, 11, 255), IM_COL32(245, 158, 11, 50), txtCol, isEn ? "Loop End" : "Döngü Sonu" };
            case NodeKind::SwitchCase:
                return { IM_COL32(32, 24, 14, 240), IM_COL32(217, 119, 6, 255), IM_COL32(217, 119, 6, 50), txtCol, isEn ? "Switch-Case" : "Çoklu Seçim (switch)" };
            case NodeKind::StructDef:
                return { IM_COL32(34, 22, 12, 240), IM_COL32(217, 119, 6, 255), IM_COL32(217, 119, 6, 50), txtCol, isEn ? "Struct Definition" : "Özel Yapı (struct)" };
            default:
                return { IM_COL32(18, 20, 40, 240), IM_COL32(99, 102, 241, 255), IM_COL32(99, 102, 241, 50), txtCol, isEn ? "Data Structure" : "Veri Yapısı (Koleksiyon)" };
        }
    }
}

inline std::string GetNodeLocalizedTitle(const FlowNode& n, bool isEn) {
    if (n.kind == NodeKind::Start) return isEn ? "Program Start" : "Yazılımın Başlangıcı";
    if (n.kind == NodeKind::End) return isEn ? "Program End (return 0)" : "Yazılımın Sonlanışı";
    if (n.kind == NodeKind::LoopEnd) return isEn ? "Loop End" : "Döngü Sonu";
    return n.title;
}

inline std::string FormatNodeLiveContent(const FlowNode& n) {
    switch (n.kind) {
        case NodeKind::Start:
            return "int main() { ... }";
        case NodeKind::End:
            return "return 0;";
        case NodeKind::LoopEnd:
            return "}";
        case NodeKind::VariableDeclare: {
            if (!n.multiAssignments.empty()) {
                std::string s;
                for (size_t i = 0; i < n.multiAssignments.size(); i++) {
                    if (i > 0) s += ", ";
                    s += n.multiAssignments[i].varName + " = " + n.multiAssignments[i].valueExpr;
                }
                return s;
            }
            return n.targetVar + " = " + n.expr;
        }
        case NodeKind::Assign: {
            if (!n.multiAssignments.empty()) {
                std::string s;
                for (size_t i = 0; i < n.multiAssignments.size(); i++) {
                    if (i > 0) s += ", ";
                    s += n.multiAssignments[i].varName + " = " + n.multiAssignments[i].valueExpr;
                }
                return s;
            }
            return n.targetVar + " = " + n.expr;
        }
        case NodeKind::MathOp: {
            if (n.mathOp == "sqrt") return n.mathTarget + " = sqrt(" + n.mathLeft + ")";
            if (n.mathOp == "fact") return n.mathTarget + " = fact(" + n.mathLeft + ")";
            return n.mathTarget + " = " + n.mathLeft + " " + n.mathOp + " " + n.mathRight;
        }
        case NodeKind::Input:
            return "cin >> " + n.targetVar;
        case NodeKind::Print: {
            if (n.printMode == 0) return "cout << \"" + n.ioText + "\"";
            if (n.printMode == 1) return "cout << " + n.targetVar;
            return "cout << \"" + n.ioText + "\" << " + n.targetVar;
        }
        case NodeKind::BranchIf:
            return n.conditionExpr.empty() ? "x < 10 ?" : (n.conditionExpr + " ?");
        case NodeKind::LoopWhile:
            return "while (" + n.conditionExpr + ")";
        case NodeKind::LoopDoWhile:
            return "do { ... } while (" + n.conditionExpr + ")";
        case NodeKind::LoopFor:
            return "for (" + n.loopInit + "; " + n.conditionExpr + "; " + n.loopStep + ")";
        case NodeKind::SwitchCase:
            return "switch (" + n.switchVar + ")";
        case NodeKind::StructDef:
            return "struct " + n.structName + " (" + std::to_string(n.structMembers.size()) + " alan)";
        case NodeKind::VectorOp:
        case NodeKind::ListOp:
        case NodeKind::StackOp:
        case NodeKind::QueueOp:
        case NodeKind::ArrayOp:
            return n.dsName + "." + n.dsOpType + "(" + n.dsOpArg + ")";
        default:
            return n.title;
    }
}

// Canlı Hata & Bağlantı Denetimi (Live Flow Linter)
inline void UpdateFlowDiagnostics(ProjectManager& pm) {
    bool isEn = (pm.currentLanguage == AppLanguage::English);

    for (auto& n : pm.nodes) {
        n.hasWarning = false;
        n.warningText.clear();

        // 1. Start haricinde Giriş pini (pin 0) bağlı mı?
        if (n.kind != NodeKind::Start) {
            bool hasIn = false;
            for (const auto& w : pm.wires) {
                if ((w.toNode == n.id && w.toPin == 0) || (w.fromNode == n.id && w.fromPin == 0)) {
                    hasIn = true;
                    break;
                }
            }
            if (!hasIn) {
                n.hasWarning = true;
                n.warningText = isEn ? "Input connection missing" : "Giriş bağlantısı eksik";
            }
        }

        // 2. End haricinde Çıkış pini bağlı mı?
        if (!n.hasWarning && n.kind != NodeKind::End) {
            bool hasOut = false;
            for (const auto& w : pm.wires) {
                if (n.kind == NodeKind::Start) {
                    if ((w.fromNode == n.id && w.fromPin == 0) || (w.toNode == n.id && w.toPin == 0)) {
                        hasOut = true;
                        break;
                    }
                } else {
                    if ((w.fromNode == n.id && w.fromPin > 0) || (w.toNode == n.id && w.toPin > 0)) {
                        hasOut = true;
                        break;
                    }
                }
            }
            if (!hasOut) {
                n.hasWarning = true;
                n.warningText = isEn ? "Output connection missing" : "Çıkış bağlantısı eksik";
            }
        }

        // 3. Döngü Şemalarında Gövde (Body - Pin 1) Bağlı mı?
        if (!n.hasWarning && (n.kind == NodeKind::LoopWhile || n.kind == NodeKind::LoopDoWhile || n.kind == NodeKind::LoopFor)) {
            bool hasBody = false;
            for (const auto& w : pm.wires) {
                if (w.fromNode == n.id && w.fromPin == 1) {
                    hasBody = true;
                    break;
                }
            }
            if (!hasBody) {
                n.hasWarning = true;
                n.warningText = isEn ? "Loop body connection missing" : "Döngü gövdesi bağlantısı eksik";
            }
        }

        // 4. Switch-Case Kuralı: String metin kabul edilemez!
        if (!n.hasWarning && n.kind == NodeKind::SwitchCase) {
            bool isStringSwitch = false;
            for (const auto& other : pm.nodes) {
                if (other.kind == NodeKind::VariableDeclare && other.targetVar == n.switchVar && other.varType == TypeKind::String) {
                    isStringSwitch = true;
                    break;
                }
            }
            if (!n.switchVar.empty() && (n.switchVar.front() == '"' || n.switchVar.back() == '"')) {
                isStringSwitch = true;
            }
            if (isStringSwitch) {
                n.hasWarning = true;
                n.warningText = isEn ? "C++ Error: switch cannot evaluate string!" : "C++ Kuralı: switch içine metin (string) yazılamaz!";
            }
        }
    }
}

// ── 2. Akis Semasi Grafigi (Flow Graph Workspace) ────────────────────────
void RenderFlowGraphWorkspace(ProjectManager& pm, int& selectedNodeId) {
    bool isEn = (pm.currentLanguage == AppLanguage::English);

    ImGui::BeginChild("FlowToolbar", ImVec2(0, 44), true);
    if (ImGui::Button(isEn ? "+ Variable" : "+ Değişken")) {
        pm.AddNode(NodeKind::VariableDeclare, isEn ? "Variable Declare" : "Değişken Tanımla", 150, 150);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "+ Struct" : "+ Özel Yapı (Struct)")) {
        pm.AddNode(NodeKind::StructDef, isEn ? "Struct Definition" : "Özel Yapı (Struct)", 180, 170);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "+ Data Structures v" : "+ Veri Yapıları v")) {
        ImGui::OpenPopup("ToolbarDataStructuresPopup");
    }
    if (ImGui::BeginPopup("ToolbarDataStructuresPopup")) {
        ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "Data Structures:" : "Veri Yapıları:");
        ImGui::Separator();
        if (ImGui::MenuItem("std::vector (Dinamik Dizi)")) {
            pm.AddNode(NodeKind::VectorOp, "std::vector", 210, 190);
        }
        if (ImGui::MenuItem("std::list (Bağlı Liste)")) {
            pm.AddNode(NodeKind::ListOp, "std::list", 220, 200);
        }
        if (ImGui::MenuItem("std::stack (Yığın - LIFO)")) {
            pm.AddNode(NodeKind::StackOp, "std::stack", 230, 210);
        }
        if (ImGui::MenuItem("std::queue (Kuyruk - FIFO)")) {
            pm.AddNode(NodeKind::QueueOp, "std::queue", 240, 220);
        }
        if (ImGui::MenuItem("Sabit Dizi (std::array / Array)")) {
            pm.AddNode(NodeKind::ArrayOp, "Array (Dizi)", 250, 230);
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "+ Assign" : "+ Değer Ata")) {
        pm.AddNode(NodeKind::Assign, isEn ? "Assign Value" : "Değer Ata", 250, 200);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "+ Math Op" : "+ Matematik")) {
        pm.AddNode(NodeKind::MathOp, isEn ? "Math Operation" : "Matematiksel İşlem", 300, 220);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "+ Print (cout)" : "+ Yazdır (cout)")) {
        pm.AddNode(NodeKind::Print, isEn ? "Print Output" : "Konsola Yazdır", 350, 250);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "Branch (If)" : "Koşul (If)")) {
        pm.AddNode(NodeKind::BranchIf, isEn ? "Branch (If/Else)" : "Koşul / Karar (if)", 400, 200);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "Switch" : "Switch-Case")) {
        pm.AddNode(NodeKind::SwitchCase, isEn ? "Switch Case" : "Çoklu Seçim (switch)", 430, 220);
    }
    ImGui::SameLine();
    if (ImGui::Button("While")) {
        pm.AddNode(NodeKind::LoopWhile, isEn ? "While Loop" : "While Döngüsü", 480, 250);
    }
    ImGui::SameLine();
    if (ImGui::Button("Do-While")) {
        pm.AddNode(NodeKind::LoopDoWhile, isEn ? "Do-While Loop" : "Do-While Döngüsü", 520, 280);
    }
    ImGui::SameLine();
    if (ImGui::Button("For")) {
        pm.AddNode(NodeKind::LoopFor, isEn ? "For Loop" : "For Döngüsü", 560, 300);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "+ Loop End" : "+ Döngü Sonu")) {
        pm.AddNode(NodeKind::LoopEnd, isEn ? "Loop End" : "Döngü Sonu", 600, 320);
    }
    ImGui::SameLine(0, 12);
    if (ImGui::Button(isEn ? "Reset View (100%)" : "Kamerayı Sıfırla (%100)")) {
        g_flowPanOffset = { 0.0f, 0.0f };
        g_flowZoomLevel = 1.0f;
    }
    ImGui::SameLine(0, 12);
    if (ImGui::Button(isEn ? "Save Project" : "Projeyi Kaydet")) {
        pm.SaveProject();
    }
    ImGui::EndChild();

    // Cizim Alani
    ImGui::BeginChild("FlowCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 screenOrigin = ImGui::GetCursorScreenPos();
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    ImVec2 size = ImGui::GetContentRegionAvail();

    bool isLight = (pm.currentTheme == AppTheme::LightPastelNeon);
    // Açık tema: şampanya-lavanta | Koyu tema: derin neon mor (Amethyst Night)
    ImU32 canvasBgCol = isLight ? IM_COL32(247, 245, 250, 255) : IM_COL32(22, 10, 44, 255);
    drawList->AddRectFilled(screenOrigin, ImVec2(screenOrigin.x + size.x, screenOrigin.y + size.y), canvasBgCol);

    bool isCanvasHovered = ImGui::IsWindowHovered();

    // 1. Orta Tus (Middle Click) veya Alt + Sol Tik ile Harita Kaydirma (Pan)
    if (isCanvasHovered && (ImGui::IsMouseDragging(2, 0.0f) || (ImGui::IsMouseDragging(0, 0.0f) && ImGui::GetIO().KeyAlt))) {
        ImVec2 dragDelta = ImGui::GetIO().MouseDelta;
        g_flowPanOffset.x += dragDelta.x;
        g_flowPanOffset.y += dragDelta.y;
    }

    // 2. Fare Tekerlegi ile Imlece Dogru Yakinlastirma / Uzaklastirma (Zoom: 0.6x - 1.6x)
    float wheel = ImGui::GetIO().MouseWheel;
    if (isCanvasHovered && wheel != 0.0f) {
        float prevZoom = g_flowZoomLevel;
        g_flowZoomLevel += wheel * 0.08f;
        if (g_flowZoomLevel < 0.6f) g_flowZoomLevel = 0.6f;
        if (g_flowZoomLevel > 1.6f) g_flowZoomLevel = 1.6f;

        float worldMouseX = (mousePos.x - screenOrigin.x - g_flowPanOffset.x) / prevZoom;
        float worldMouseY = (mousePos.y - screenOrigin.y - g_flowPanOffset.y) / prevZoom;

        g_flowPanOffset.x = (mousePos.x - screenOrigin.x) - worldMouseX * g_flowZoomLevel;
        g_flowPanOffset.y = (mousePos.y - screenOrigin.y) - worldMouseY * g_flowZoomLevel;
    }

    // 3. Sonsuz Izgara Cizimi (Sifir Ek Bellek Maliyetli Modulo Geometrisi)
    if (pm.showGrid) {
        // Açık tema: leylak-ametist | Koyu tema: leylak neon çizgi (derin mor zemin üstü görünür)
        ImU32 gridCol = isLight ? IM_COL32(228, 220, 242, 255) : IM_COL32(70, 28, 120, 255);
        float gridSize = 32.0f * g_flowZoomLevel;
        float startX = fmodf(g_flowPanOffset.x, gridSize);
        if (startX < 0.0f) startX += gridSize;
        float startY = fmodf(g_flowPanOffset.y, gridSize);
        if (startY < 0.0f) startY += gridSize;

        for (float x = startX; x < size.x; x += gridSize) {
            drawList->AddLine(ImVec2(screenOrigin.x + x, screenOrigin.y), ImVec2(screenOrigin.x + x, screenOrigin.y + size.y), gridCol);
        }
        for (float y = startY; y < size.y; y += gridSize) {
            drawList->AddLine(ImVec2(screenOrigin.x, screenOrigin.y + y), ImVec2(screenOrigin.x + size.x, screenOrigin.y + y), gridCol);
        }
    }

    // Klavyeden Delete tusuna basilinca secili dugumu sil
    if (selectedNodeId > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        pm.DeleteNode(selectedNodeId);
        selectedNodeId = 0;
    }

    // ── Etkilesim Durumlari ──────────────────────────────────────────────
    static int draggingId = -1;
    static ImVec2 dragOffset = { 0, 0 };

    // Pin Baglanti Surukleme (Wiring State)
    static int wireDragFromNode = -1;
    static int wireDragFromPin = -1;
    static ImVec2 wireDragStartPos = { 0, 0 };

    // Coklu Secim (Marquee Box Selection)
    static bool isBoxSelecting = false;
    static ImVec2 boxSelectStart = { 0, 0 };
    static std::set<int> multiSelectedNodes;

    // Cift Tik ile Duzenleme Modali Durumu
    static int modalEditingNodeId = -1;

    // Sag Tik Menusu Icin Tiklanan Nokta
    static ImVec2 contextMenuPos = { 0, 0 };

    int hoveredPinNode = -1;
    int hoveredPinIdx = -1;

    // 0. Pin ve Geometri Senkronizasyonu & Canlı Hata Denetimi
    for (auto& n : pm.nodes) {
        SyncNodeGeometryAndPins(n);
    }
    UpdateFlowDiagnostics(pm);

    // ── ADIM 1: DUGUM GOVDELERI VE NEON ISIMALARI ─────────────────────────
    float animPulse = sinf((float)ImGui::GetTime() * 4.5f) * 0.35f + 0.65f;
    ImU32 warnNeonCol = IM_COL32(255, (int)(70 * animPulse), (int)(50 * animPulse), (int)(255 * animPulse));

    for (auto& n : pm.nodes) {
        ImVec2 pMin(screenOrigin.x + g_flowPanOffset.x + n.x * g_flowZoomLevel,
                    screenOrigin.y + g_flowPanOffset.y + n.y * g_flowZoomLevel);
        ImVec2 pMax(pMin.x + n.width * g_flowZoomLevel,
                    pMin.y + n.height * g_flowZoomLevel);

        bool isSelected = (selectedNodeId == n.id || multiSelectedNodes.count(n.id) > 0);
        NodeColorTheme theme = GetNodeTheme(n.kind, pm.currentTheme, isEn);
        FlowchartShape shape = GetFlowchartShape(n.kind);

        ImU32 borderCol = n.hasWarning ? warnNeonCol : (isSelected ? IM_COL32(0, 240, 255, 255) : theme.neonCol);
        float borderThick = isSelected ? 2.6f : 1.4f;

        switch (shape) {
            case FlowchartShape::Pill: {
                float r = (pMax.y - pMin.y) * 0.5f;
                drawList->AddRectFilled(pMin, pMax, theme.bgCol, r);
                DrawNeonRect(drawList, pMin, pMax, borderCol, r, borderThick);
                break;
            }
            case FlowchartShape::Diamond: {
                ImVec2 mid((pMin.x + pMax.x) * 0.5f, (pMin.y + pMax.y) * 0.5f);
                ImVec2 pts[4] = {
                    ImVec2(mid.x, pMin.y),
                    ImVec2(pMax.x, mid.y),
                    ImVec2(mid.x, pMax.y),
                    ImVec2(pMin.x, mid.y)
                };
                drawList->AddQuadFilled(pts[0], pts[1], pts[2], pts[3], theme.bgCol);
                DrawNeonPolyline(drawList, pts, 4, borderCol, borderThick, true);
                break;
            }
            case FlowchartShape::Parallelogram: {
                float skew = 18.0f * g_flowZoomLevel;
                ImVec2 pts[4] = {
                    ImVec2(pMin.x + skew, pMin.y),
                    ImVec2(pMax.x, pMin.y),
                    ImVec2(pMax.x - skew, pMax.y),
                    ImVec2(pMin.x, pMax.y)
                };
                drawList->AddQuadFilled(pts[0], pts[1], pts[2], pts[3], theme.bgCol);
                DrawNeonPolyline(drawList, pts, 4, borderCol, borderThick, true);
                break;
            }
            case FlowchartShape::Hexagon: {
                float midY = (pMin.y + pMax.y) * 0.5f;
                float inset = 20.0f * g_flowZoomLevel;
                ImVec2 pts[6] = {
                    ImVec2(pMin.x + inset, pMin.y),
                    ImVec2(pMax.x - inset, pMin.y),
                    ImVec2(pMax.x, midY),
                    ImVec2(pMax.x - inset, pMax.y),
                    ImVec2(pMin.x + inset, pMax.y),
                    ImVec2(pMin.x, midY)
                };
                drawList->AddConvexPolyFilled(pts, 6, theme.bgCol);
                DrawNeonPolyline(drawList, pts, 6, borderCol, borderThick, true);
                break;
            }
            case FlowchartShape::Rectangle:
            default: {
                float r = 8.0f * g_flowZoomLevel;
                drawList->AddRectFilled(pMin, pMax, theme.bgCol, r);
                drawList->AddRectFilled(pMin, ImVec2(pMax.x, pMin.y + 24.0f * g_flowZoomLevel), theme.headCol, r, ImDrawFlags_RoundCornersTop);
                DrawNeonRect(drawList, pMin, pMax, borderCol, r, borderThick);
                break;
            }
        }
    }

    // ── ADIM 2: BEZIER KABLOLARI (OK BASLI VE SOKET NOKTALI AKIS) ────────
    // Not: Kablolar dugum zeminlerinin ustunde, ancak metinlerin altinda cizilir.
    // Başlangıç noktasında dairesel soket (•), hedef noktasında ise ok başı (▶) yer alır!
    int wireToDelete = -1;
    bool isAltDown = ImGui::GetIO().KeyAlt;

    for (const auto& w : pm.wires) {
        FlowNode* fn = pm.GetNode(w.fromNode);
        FlowNode* tn = pm.GetNode(w.toNode);
        if (fn && tn && w.fromPin < (int)fn->pins.size() && w.toPin < (int)tn->pins.size()) {
            ImVec2 p1(screenOrigin.x + g_flowPanOffset.x + (fn->x + fn->pins[w.fromPin].relX) * g_flowZoomLevel,
                      screenOrigin.y + g_flowPanOffset.y + (fn->y + fn->pins[w.fromPin].relY) * g_flowZoomLevel);
            ImVec2 p2(screenOrigin.x + g_flowPanOffset.x + (tn->x + tn->pins[w.toPin].relX) * g_flowZoomLevel,
                      screenOrigin.y + g_flowPanOffset.y + (tn->y + tn->pins[w.toPin].relY) * g_flowZoomLevel);

            float dx = fabsf(p2.x - p1.x) * 0.5f;
            if (dx < 40.0f * g_flowZoomLevel) dx = 40.0f * g_flowZoomLevel;

            bool isBottomPin = (fn->kind == NodeKind::BranchIf && w.fromPin == 2);
            float dy = fabsf(p2.y - p1.y) * 0.5f;
            if (dy < 40.0f * g_flowZoomLevel) dy = 40.0f * g_flowZoomLevel;

            ImVec2 c1 = isBottomPin ? ImVec2(p1.x, p1.y + dy) : ImVec2(p1.x + dx, p1.y);
            ImVec2 c2(p2.x - dx, p2.y);

            ImVec2 midPt((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);
            float distToMid = hypotf(mousePos.x - midPt.x, mousePos.y - midPt.y);
            bool wireHovered = (distToMid < 25.0f * g_flowZoomLevel);

            if (wireHovered && isAltDown) {
                drawList->AddBezierCubic(p1, c1, c2, p2, IM_COL32(255, 60, 60, 255), 4.5f);
                drawList->AddCircleFilled(midPt, 7.0f, IM_COL32(255, 30, 30, 255));
                drawList->AddText(ImVec2(midPt.x + 10, midPt.y - 10), IM_COL32(255, 100, 100, 255), isEn ? "[Cut Wire]" : "[Kabloyu Kes]");
                if (ImGui::IsMouseClicked(0)) {
                    wireToDelete = w.id;
                }
            } else {
                // 1. Kaynak Soket Noktasi (Solid Socket Dot)
                ImU32 dotCol = isBottomPin ? IM_COL32(239, 68, 68, 255) : (isLight ? IM_COL32(15, 118, 110, 255) : IM_COL32(0, 220, 255, 255));
                drawList->AddCircleFilled(p1, 4.0f * g_flowZoomLevel, dotCol);
                drawList->AddCircle(p1, 5.5f * g_flowZoomLevel, IM_COL32(255, 255, 255, 220), 0, 1.2f);

                // 2. Lazer Bézier Teli
                ImU32 wireGlow = isBottomPin ? (isLight ? IM_COL32(244, 63, 94, 60) : IM_COL32(240, 90, 90, 60))
                                             : (isLight ? IM_COL32(20, 184, 166, 60) : IM_COL32(0, 180, 240, 65));
                ImU32 wireCore = isBottomPin ? (isLight ? IM_COL32(225, 29, 72, 255) : IM_COL32(255, 120, 90, 255))
                                             : (isLight ? IM_COL32(13, 148, 136, 255) : IM_COL32(0, 230, 255, 255));
                drawList->AddBezierCubic(p1, c1, c2, p2, wireGlow, 5.5f);
                drawList->AddBezierCubic(p1, c1, c2, p2, wireCore, 2.4f);

                // 3. Hedef Ok Başı (Target Arrowhead) - Teğet doğrultusunda tam p2'ye saplanır!
                DrawWireArrowhead(drawList, p2, ImVec2(p2.x - c2.x, p2.y - c2.y), 11.0f * g_flowZoomLevel, wireCore);
            }
        }
    }

    if (wireToDelete > 0) {
        pm.DisconnectWire(wireToDelete);
        pm.statusMessage = isEn ? "Wire connection removed." : "Kablo baglantisi koparildi (Alt + Tik).";
    }

    // ── ADIM 3: DUGUM ICI CANLI METIN VE IFADELER ─────────────────────────
    for (auto& n : pm.nodes) {
        ImVec2 pMin(screenOrigin.x + g_flowPanOffset.x + n.x * g_flowZoomLevel,
                    screenOrigin.y + g_flowPanOffset.y + n.y * g_flowZoomLevel);
        ImVec2 pMax(pMin.x + n.width * g_flowZoomLevel,
                    pMin.y + n.height * g_flowZoomLevel);
        ImVec2 pCenter((pMin.x + pMax.x) * 0.5f, (pMin.y + pMax.y) * 0.5f);

        NodeColorTheme theme = GetNodeTheme(n.kind, pm.currentTheme, isEn);
        FlowchartShape shape = GetFlowchartShape(n.kind);
        std::string liveExpr = FormatNodeLiveContent(n);
        if (liveExpr.length() > 32) liveExpr = liveExpr.substr(0, 29) + "...";

        // ── Metin Kırpma Kalkanı: tüm metinler şema sınırı içinde kalır ──────
        // Parallelogram'da skew nedeniyle biraz daha geniş clip alanı bırakıyoruz
        float clipInset = (shape == FlowchartShape::Parallelogram) ? 0.0f : 2.0f;
        ImVec2 clipMin(pMin.x + clipInset, pMin.y + clipInset);
        ImVec2 clipMax(pMax.x - clipInset, pMax.y - clipInset);
        drawList->PushClipRect(clipMin, clipMax, true);

        // Rozet / Kategori ve Canlı İfade Çizimi
        if (shape == FlowchartShape::Diamond) {
            // BranchIf: Merkezde koşul, sağda Evet, altta Hayır
            ImVec2 szTitle = ImGui::CalcTextSize(theme.categoryBadge);
            drawList->AddText(ImVec2(pCenter.x - szTitle.x * 0.5f, pMin.y + 12.0f * g_flowZoomLevel), theme.neonCol, theme.categoryBadge);

            ImVec2 szExpr = ImGui::CalcTextSize(liveExpr.c_str());
            drawList->AddText(ImVec2(pCenter.x - szExpr.x * 0.5f, pCenter.y - szExpr.y * 0.5f), theme.textCol, liveExpr.c_str());

            // Dal İpuçları (Evet / Hayır)
            drawList->AddText(ImVec2(pMax.x - 52.0f * g_flowZoomLevel, pCenter.y - 18.0f), IM_COL32(16, 185, 129, 255), isEn ? "True" : "Evet");
            drawList->AddText(ImVec2(pCenter.x + 8.0f, pMax.y - 24.0f * g_flowZoomLevel), IM_COL32(239, 68, 68, 255), isEn ? "False" : "Hayır");
        } else if (shape == FlowchartShape::Pill) {
            // Start / End / LoopEnd
            std::string titleStr = GetNodeLocalizedTitle(n, isEn);
            ImVec2 szTitle = ImGui::CalcTextSize(titleStr.c_str());
            drawList->AddText(ImVec2(pCenter.x - szTitle.x * 0.5f, pCenter.y - 14.0f), theme.textCol, titleStr.c_str());
            ImVec2 szExpr = ImGui::CalcTextSize(liveExpr.c_str());
            drawList->AddText(ImVec2(pCenter.x - szExpr.x * 0.5f, pCenter.y + 2.0f), theme.neonCol, liveExpr.c_str());
        } else if (shape == FlowchartShape::Hexagon) {
            // Döngüler (While, Do-While, For): Başlık + Canlı Koşul (Gövde etiketi kaldırıldı)
            float titleY = pMin.y + 7.0f * g_flowZoomLevel;
            float textIndent = 24.0f * g_flowZoomLevel;

            drawList->AddText(ImVec2(pMin.x + textIndent, titleY), theme.neonCol, theme.categoryBadge);

            float bodyY = pCenter.y - 4.0f * g_flowZoomLevel;
            drawList->AddText(ImVec2(pMin.x + textIndent, bodyY), theme.textCol, liveExpr.c_str());
            // NOT: "Gövde"/"Body" etiketi kasıtlı olarak kaldırıldı — pin zaten anlamlı.
        } else {
            // Parallelogram, Rectangle
            float titleY = pMin.y + 6.0f * g_flowZoomLevel;
            float textIndent = (shape == FlowchartShape::Parallelogram) ? (24.0f * g_flowZoomLevel) : (18.0f * g_flowZoomLevel);

            drawList->AddText(ImVec2(pMin.x + textIndent, titleY), theme.neonCol, theme.categoryBadge);

            float bodyY = pCenter.y - 3.0f * g_flowZoomLevel;
            drawList->AddText(ImVec2(pMin.x + textIndent, bodyY), theme.textCol, liveExpr.c_str());
        }

        // Canlı Uyarı Rozeti
        if (n.hasWarning) {
            std::string warnStr = "[!] " + n.warningText;
            drawList->AddText(ImVec2(pMin.x + 14.0f * g_flowZoomLevel, pMax.y - 16.0f * g_flowZoomLevel), warnNeonCol, warnStr.c_str());
        }

        drawList->PopClipRect(); // Metin kırpma kalkanı sonu
    }

    // ── ADIM 4: PINLER VE ETKİLEŞİM (YAZISIZ TEMİZ SOKETLER & ÇİFT TIK) ───
    for (auto& n : pm.nodes) {
        ImVec2 pMin(screenOrigin.x + g_flowPanOffset.x + n.x * g_flowZoomLevel,
                    screenOrigin.y + g_flowPanOffset.y + n.y * g_flowZoomLevel);
        ImVec2 pMax(pMin.x + n.width * g_flowZoomLevel,
                    pMin.y + n.height * g_flowZoomLevel);

        bool isNodeHovered = (mousePos.x >= pMin.x && mousePos.x <= pMax.x && mousePos.y >= pMin.y && mousePos.y <= pMax.y);

        for (size_t pIdx = 0; pIdx < n.pins.size(); pIdx++) {
            const auto& pin = n.pins[pIdx];
            ImVec2 pinPos(pMin.x + pin.relX * g_flowZoomLevel, pMin.y + pin.relY * g_flowZoomLevel);

            float distToPin = hypotf(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
            bool isPinHovered = (distToPin <= 10.0f * g_flowZoomLevel);

            if (isPinHovered) {
                hoveredPinNode = n.id;
                hoveredPinIdx = (int)pIdx;
            }

            // Temiz Soket Çizimi (Giriş/Çıkış metinleri silindi, minimal ve şık)
            if (pin.type == PinType::ExecIn) {
                // Giriş docking noktası
                drawList->AddCircleFilled(pinPos, 3.5f * g_flowZoomLevel, isPinHovered ? IM_COL32(255, 230, 80, 255) : (isLight ? IM_COL32(148, 163, 184, 255) : IM_COL32(100, 116, 139, 255)));
                drawList->AddCircle(pinPos, 4.5f * g_flowZoomLevel, IM_COL32(255, 255, 255, isPinHovered ? 255 : 120));
            } else {
                // Çıkış soket noktası
                ImU32 pCol = (n.kind == NodeKind::BranchIf && pIdx == 2) ? IM_COL32(239, 68, 68, 255) : (isLight ? IM_COL32(15, 118, 110, 255) : IM_COL32(0, 210, 255, 255));
                if (isPinHovered) pCol = IM_COL32(255, 220, 50, 255);
                drawList->AddCircleFilled(pinPos, 4.5f * g_flowZoomLevel, pCol);
                drawList->AddCircle(pinPos, 6.0f * g_flowZoomLevel, IM_COL32(255, 255, 255, isPinHovered ? 255 : 160));
            }
        }

        // Dugum Surukleme, Secim ve CIFT TIK ILE MODAL ACMA Tetikleyicisi
        if (isNodeHovered && hoveredPinNode == -1) {
            ImGui::SetTooltip(isEn 
                ? "💡 Double-click to configure Algorithm Schema properties." 
                : "💡 Çift tıklayarak Algoritma Şeması ayarlarını düzenleyebilirsiniz.");

            if (ImGui::IsMouseDoubleClicked(0)) {
                modalEditingNodeId = n.id;
            } else if (ImGui::IsMouseClicked(0)) {
                selectedNodeId = n.id;
                multiSelectedNodes.clear();
                multiSelectedNodes.insert(n.id);
                draggingId = n.id;
                dragOffset = ImVec2(mousePos.x - pMin.x, mousePos.y - pMin.y);
            }
        }

        if (draggingId == n.id) {
            if (ImGui::IsMouseDown(0)) {
                float targetScreenX = mousePos.x - dragOffset.x;
                float targetScreenY = mousePos.y - dragOffset.y;
                n.x = (targetScreenX - screenOrigin.x - g_flowPanOffset.x) / g_flowZoomLevel;
                n.y = (targetScreenY - screenOrigin.y - g_flowPanOffset.y) / g_flowZoomLevel;
            } else {
                draggingId = -1;
            }
        }
    }

    // ── 2. Pinden Canli Kablo Cekme Etkilesimi (Ok Başlı Canlı Çekim) ──────
    if (ImGui::IsMouseClicked(0) && hoveredPinNode > 0) {
        FlowNode* fn = pm.GetNode(hoveredPinNode);
        if (fn && hoveredPinIdx < (int)fn->pins.size()) {
            wireDragFromNode = hoveredPinNode;
            wireDragFromPin = hoveredPinIdx;
            wireDragStartPos = ImVec2(screenOrigin.x + g_flowPanOffset.x + (fn->x + fn->pins[hoveredPinIdx].relX) * g_flowZoomLevel, 
                                      screenOrigin.y + g_flowPanOffset.y + (fn->y + fn->pins[hoveredPinIdx].relY) * g_flowZoomLevel);
        }
    }

    if (wireDragFromNode > 0) {
        if (ImGui::IsMouseDown(0)) {
            FlowNode* srcNode = pm.GetNode(wireDragFromNode);
            bool isBottomSrc = (srcNode && srcNode->kind == NodeKind::BranchIf && wireDragFromPin == 2);
            float dx = fabsf(mousePos.x - wireDragStartPos.x) * 0.5f;
            if (dx < 40.0f * g_flowZoomLevel) dx = 40.0f * g_flowZoomLevel;
            float dy = fabsf(mousePos.y - wireDragStartPos.y) * 0.5f;
            if (dy < 40.0f * g_flowZoomLevel) dy = 40.0f * g_flowZoomLevel;

            ImVec2 c1 = isBottomSrc ? ImVec2(wireDragStartPos.x, wireDragStartPos.y + dy) : ImVec2(wireDragStartPos.x + dx, wireDragStartPos.y);
            ImVec2 c2(mousePos.x - dx, mousePos.y);

            drawList->AddCircleFilled(wireDragStartPos, 4.5f * g_flowZoomLevel, IM_COL32(245, 158, 11, 255));
            drawList->AddBezierCubic(wireDragStartPos, c1, c2, mousePos, IM_COL32(245, 158, 11, 70), 5.5f);
            drawList->AddBezierCubic(wireDragStartPos, c1, c2, mousePos, IM_COL32(255, 200, 50, 255), 2.5f);

            // Farenin ucunda yönü gösteren dinamik ok başı!
            DrawWireArrowhead(drawList, mousePos, ImVec2(mousePos.x - c2.x, mousePos.y - c2.y), 12.0f * g_flowZoomLevel, IM_COL32(255, 210, 60, 255));
        } else {
            if (hoveredPinNode > 0 && hoveredPinNode != wireDragFromNode) {
                pm.ConnectPins(wireDragFromNode, wireDragFromPin, hoveredPinNode, hoveredPinIdx);
            }
            wireDragFromNode = -1;
            wireDragFromPin = -1;
        }
    }

    // ── 3. Bos Alan Tiklamasi: Kutu Secimi (Marquee Selection) ────────────
    if (ImGui::IsMouseClicked(0) && isCanvasHovered && draggingId == -1 && wireDragFromNode == -1 && hoveredPinNode == -1) {
        isBoxSelecting = true;
        boxSelectStart = mousePos;
        multiSelectedNodes.clear();
        selectedNodeId = 0;
    }

    if (isBoxSelecting) {
        if (ImGui::IsMouseDown(0)) {
            ImVec2 bMin(fminf(boxSelectStart.x, mousePos.x), fminf(boxSelectStart.y, mousePos.y));
            ImVec2 bMax(fmaxf(boxSelectStart.x, mousePos.x), fmaxf(boxSelectStart.y, mousePos.y));

            drawList->AddRectFilled(bMin, bMax, IM_COL32(0, 160, 255, 35));
            drawList->AddRect(bMin, bMax, IM_COL32(0, 180, 255, 200), 0.0f, 0, 1.5f);

            for (const auto& n : pm.nodes) {
                float nLeft = screenOrigin.x + g_flowPanOffset.x + n.x * g_flowZoomLevel;
                float nRight = nLeft + n.width * g_flowZoomLevel;
                float nTop = screenOrigin.y + g_flowPanOffset.y + n.y * g_flowZoomLevel;
                float nBottom = nTop + n.height * g_flowZoomLevel;

                if (nRight >= bMin.x && nLeft <= bMax.x && nBottom >= bMin.y && nTop <= bMax.y) {
                    multiSelectedNodes.insert(n.id);
                    selectedNodeId = n.id;
                }
            }
        } else {
            isBoxSelecting = false;
        }
    }

    // ── 4. Sag Tik: DUGUM EKLEME BAGLAM MENUSU (Context Spawn Menu) ───────
    if (ImGui::IsMouseClicked(1) && isCanvasHovered) {
        contextMenuPos = ImVec2((mousePos.x - screenOrigin.x - g_flowPanOffset.x) / g_flowZoomLevel,
                                (mousePos.y - screenOrigin.y - g_flowPanOffset.y) / g_flowZoomLevel);
        ImGui::OpenPopup("NodeSpawnMenu");
    }

    if (ImGui::BeginPopup("NodeSpawnMenu")) {
        ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "Spawn Algorithm Schema:" : "Algoritma Şeması Ekle:");
        ImGui::Separator();

        if (ImGui::MenuItem(isEn ? "Program Start" : "Yazılımın Başlangıcı")) {
            pm.AddNode(NodeKind::Start, isEn ? "Program Start" : "Yazılımın Başlangıcı", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "Variable Declare" : "Değişken Tanımlama")) {
            pm.AddNode(NodeKind::VariableDeclare, isEn ? "Variable Declare" : "Değişken Tanımla", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "Struct Definition" : "Özel Yapı Tanımla (Struct)")) {
            pm.AddNode(NodeKind::StructDef, isEn ? "Struct Definition" : "Özel Yapı (Struct)", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::BeginMenu(isEn ? "Data Structures" : "Veri Yapıları")) {
            if (ImGui::MenuItem("std::vector (Dinamik Dizi)")) {
                pm.AddNode(NodeKind::VectorOp, "std::vector", contextMenuPos.x, contextMenuPos.y);
            }
            if (ImGui::MenuItem("std::list (Bağlı Liste)")) {
                pm.AddNode(NodeKind::ListOp, "std::list", contextMenuPos.x, contextMenuPos.y);
            }
            if (ImGui::MenuItem("std::stack (Yığın - LIFO)")) {
                pm.AddNode(NodeKind::StackOp, "std::stack", contextMenuPos.x, contextMenuPos.y);
            }
            if (ImGui::MenuItem("std::queue (Kuyruk - FIFO)")) {
                pm.AddNode(NodeKind::QueueOp, "std::queue", contextMenuPos.x, contextMenuPos.y);
            }
            if (ImGui::MenuItem("Sabit Dizi (std::array / Array)")) {
                pm.AddNode(NodeKind::ArrayOp, "Array (Dizi)", contextMenuPos.x, contextMenuPos.y);
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem(isEn ? "Assign Value" : "Değer Atama")) {
            pm.AddNode(NodeKind::Assign, isEn ? "Assign Value" : "Değer Ata", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "Math Operation" : "Matematiksel İşlem")) {
            pm.AddNode(NodeKind::MathOp, isEn ? "Math Operation" : "Matematiksel İşlem", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "Print Output (cout)" : "Konsol Çıktısı (cout)")) {
            pm.AddNode(NodeKind::Print, isEn ? "Print Output" : "Konsola Yazdır", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "Input Read (cin)" : "Kullanıcı Girdisi (cin)")) {
            pm.AddNode(NodeKind::Input, isEn ? "Input Read" : "Klavyeden Girdi", contextMenuPos.x, contextMenuPos.y);
        }
        ImGui::Separator();
        if (ImGui::MenuItem(isEn ? "Branch Condition (if/else)" : "Koşul / Karar (if/else)")) {
            pm.AddNode(NodeKind::BranchIf, isEn ? "Branch (If/Else)" : "Koşul / Karar (if)", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "Switch Case" : "Çoklu Seçim (switch)")) {
            pm.AddNode(NodeKind::SwitchCase, isEn ? "Switch Case" : "Çoklu Seçim (switch)", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "While Loop" : "While Döngüsü")) {
            pm.AddNode(NodeKind::LoopWhile, isEn ? "While Loop" : "While Döngüsü", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "Do-While Loop" : "Do-While Döngüsü")) {
            pm.AddNode(NodeKind::LoopDoWhile, isEn ? "Do-While Loop" : "Do-While Döngüsü", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "For Loop" : "For Döngüsü")) {
            pm.AddNode(NodeKind::LoopFor, isEn ? "For Loop" : "For Döngüsü", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem(isEn ? "Loop End" : "Döngü Sonu")) {
            pm.AddNode(NodeKind::LoopEnd, isEn ? "Loop End" : "Döngü Sonu", contextMenuPos.x, contextMenuPos.y);
        }
        ImGui::Separator();
        if (ImGui::MenuItem(isEn ? "Program End (return 0)" : "Yazılımın Sonlanışı (return 0)")) {
            pm.AddNode(NodeKind::End, isEn ? "Program End (return 0)" : "Yazılımın Sonlanışı", contextMenuPos.x, contextMenuPos.y);
        }

        ImGui::EndPopup();
    }

    ImGui::EndChild();

    // ── 5. CIFT TIK ILE ACILAN ORTALANMIS ALGORİTMA ŞEMASI DÜZENLEME MODALI ─
    if (modalEditingNodeId > 0) {
        FlowNode* editNode = pm.GetNode(modalEditingNodeId);
        if (editNode) {
            ImGui::OpenPopup("NodeEditModal");
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(620, 520), ImGuiCond_Appearing);

            bool isOpen = true;
            if (ImGui::BeginPopupModal("NodeEditModal", &isOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextColored(ImVec4(0.05f, 0.58f, 0.53f, 1.0f), isEn ? "ALGORITHM SCHEMA EDITOR & INSPECTOR" : "Algoritma Şeması Düzenleyicisi ve Ayarları");
                ImGui::TextColored(ImVec4(0.45f, 0.52f, 0.60f, 1.0f), isEn ? "Configure schema values, variables, and logic." : "Şema değerlerini, değişkenleri ve mantıksal akışı düzenleyin.");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::BeginChild("ModalScrollArea", ImVec2(580, 350), true);
                RenderInspectorPanel(pm, modalEditingNodeId);
                ImGui::EndChild();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button(isEn ? "Save & Close" : "Kaydet ve Kapat", ImVec2(160, 36)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                    modalEditingNodeId = -1;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.20f, 0.20f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.30f, 0.30f, 1.0f));
                if (ImGui::Button(isEn ? "Delete Schema" : "Algoritma Şemasını Sil", ImVec2(170, 36))) {
                    pm.DeleteNode(editNode->id);
                    modalEditingNodeId = -1;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine();
                if (ImGui::Button(isEn ? "Cancel (Esc)" : "İptal (Esc)", ImVec2(100, 36)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    modalEditingNodeId = -1;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            } else {
                modalEditingNodeId = -1;
            }
        } else {
            modalEditingNodeId = -1;
        }
    }
}

// ── 3. Data Structure Advisor Workspace ──────────────────────────────────
void RenderAdvisorWorkspace(ProjectManager& pm) {
    ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), "🧠 C-ALGORITHM VERI YAPISI DANISMANI & KARMASIKLIK ANALIZI");
    ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), "Birlikte tasarladigimiz statik kurallar akisin CPU & RAM darbogazlarini yakalar.");
    ImGui::Separator();

    ImGui::BeginChild("AdvisorControls", ImVec2(0, 110), true);
    ImGui::Columns(3, "AdvisorCols", false);

    ImGui::Text("1. Veri Yapisi:");
    const char* dsNames[] = { "std::vector (Dizi)", "std::list (Bagli Liste)", "std::stack (Yigin)", "std::queue (Kuyruk)", "std::set / BinaryTree (Agac)" };
    int currentDs = (int)pm.advisorDS;
    if (ImGui::Combo("##ds", &currentDs, dsNames, IM_ARRAYSIZE(dsNames))) {
        pm.advisorDS = (DataStructureType)currentDs;
        pm.RunDiagnostics();
    }

    ImGui::NextColumn();

    ImGui::Text("2. Yapilan Islem:");
    const char* opNames[] = { "Sona Ekle (PushBack)", "Bastan Sil (PopFront)", "Ortaya Ekle (InsertMiddle)", "Indeksle Oku (data[i])", "Eleman Ara (Search)" };
    int currentOp = (int)pm.advisorOp;
    if (ImGui::Combo("##op", &currentOp, opNames, IM_ARRAYSIZE(opNames))) {
        pm.advisorOp = (OperationType)currentOp;
        pm.RunDiagnostics();
    }

    ImGui::NextColumn();

    ImGui::Text("3. Eleman Sayisi (N):");
    if (ImGui::SliderInt("##nval", &pm.advisorCount, 10, 1000000, "%d Eleman")) {
        pm.RunDiagnostics();
    }

    ImGui::Columns(1);
    ImGui::EndChild();

    // Kartlar
    ImGui::BeginChild("AdvisorCards", ImVec2(0, 0), true);
    for (const auto& msg : pm.advisorMessages) {
        ImVec4 col = (msg.level == DiagnosticMessage::Error)   ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) :
                     (msg.level == DiagnosticMessage::Warning) ? ImVec4(1.0f, 0.75f, 0.2f, 1.0f) :
                                                                 ImVec4(0.3f, 0.9f, 0.5f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Border, col);
        ImGui::BeginChild(("card_" + msg.title).c_str(), ImVec2(0, 180), true);

        ImGui::TextColored(col, "%s", msg.title.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - 250);
        ImGui::TextColored(ImVec4(0, 0.8f, 1.0f, 1.0f), "Karmasiklik: %s", msg.timeComplexity.c_str());
        ImGui::Separator();

        // Teshis
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "🔍 Teshis:");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", msg.diagnosis.c_str());

        // Neden Darbogaz?
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.4f, 1.0f), "🧠 Neden Darbogaz?");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", msg.explanation.c_str());

        // Recete / Tavsiye
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.5f, 1.0f), "💡 Cozum Recetesi:");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", msg.prescription.c_str());

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    ImGui::EndChild();
}

// ── 4. Canli C++ Kodu + Build & Run Terminal Workspace ───────────────────
// Derleyici ciktisini ve program ciktisini tampon'da tutan global buffer
static std::string g_buildOutput;
static bool        g_buildRunning  = false;
static bool        g_showGuide     = false;
static bool        g_lastBuildOk   = false;
static char        g_tempCppPath[MAX_PATH] = {};
static char        g_tempExePath[MAX_PATH] = {};

// MSVC vcvars64.bat konumu (sabit yol — compile.bat ile ayni)
static const char* VCVARS_PATH =
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\"
    "VC\\Auxiliary\\Build\\vcvars64.bat";

// Proje klasoru — .exe ve .cpp gecici dosyalar buraya kaydedilir
static const char* PROJECTS_DIR =
    "C:\\Users\\ASUS TUF GAMING\\Desktop\\C-Algorithm\\projects";

// cl.exe mevcutlugunu kontrol eden yardimci
static bool DetectMSVC() {
    FILE* f = nullptr;
    fopen_s(&f, VCVARS_PATH, "r");
    if (f) { fclose(f); return true; }
    return false;
}

// Komut satirini calistirip butun ciktisini string olarak donduran yardimci
static std::string RunCommandCapture(const std::string& cmd, bool& outOk) {
    std::string result;
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) { outOk = false; return "[HATA] _popen basarisiz oldu!\n"; }
    char buf[512];
    while (fgets(buf, sizeof(buf), pipe)) {
        result += buf;
    }
    int exitCode = _pclose(pipe);
    outOk = (exitCode == 0);
    result += "\n[Cikis Kodu: " + std::to_string(exitCode) + "]\n";
    return result;
}

void RenderCodegenWorkspace(ProjectManager& pm) {
    static int selectedLang = 0; // 0: C++17, 1: ANSI C (C99)
    bool isEn = (pm.currentLanguage == AppLanguage::English);

    std::string cppCode = pm.GenerateCppCode();
    std::string cCode = pm.GenerateCCode();
    std::string currentCode = (selectedLang == 0) ? cppCode : cCode;

    // ── Ust arac cubugu ───────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.07f, 0.10f, 1.0f));
    ImGui::BeginChild("CodegenBar", ImVec2(0, 46), true);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);

    // Dil Seçim Butonları (Modern C++17 vs Saf ANSI C)
    if (selectedLang == 0) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.65f, 0.56f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.10f, 0.28f, 1.0f));
    }
    if (ImGui::Button("⚡ Modern C++ (C++17)", ImVec2(165, 30))) {
        selectedLang = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (selectedLang == 1) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.65f, 0.56f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.10f, 0.28f, 1.0f));
    }
    if (ImGui::Button("☕ Standart ANSI C (C99)", ImVec2(175, 30))) {
        selectedLang = 1;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine(0, 15);

    // Kopyala dugmesi
    if (ImGui::Button(isEn ? "Copy Code" : "Kodu Kopyala", ImVec2(95, 30))) {
        ImGui::SetClipboardText(currentCode.c_str());
        pm.statusMessage = (selectedLang == 0) 
            ? (isEn ? "C++ code copied!" : "C++ Kodu panoya kopyalandı!")
            : (isEn ? "C code copied!" : "C Kodu panoya kopyalandı!");
    }
    ImGui::SameLine();

    // Derle & Dahili Calistir dugmesi
    if (!g_buildRunning) {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.50f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.72f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.05f, 0.38f, 0.12f, 1.0f));
        bool startBuild = ImGui::Button(
            isEn ? "Build & Run (In-App)" : "Derle & Calistir (Dahili)",
            ImVec2(190, 30));
        ImGui::PopStyleColor(3);

        if (startBuild) {
            if (!DetectMSVC()) {
                g_showGuide   = true;
                g_lastBuildOk = false;
                g_buildOutput = isEn
                    ? "[HATA] MSVC (cl.exe) bulunamadi!\nLutfen asagidaki Gereksinimler Rehberine bakin.\n"
                    : "[HATA] MSVC (cl.exe) bulunamadi!\nLutfen asagidaki Gereksinimler Rehberine bakin.\n";
            } else {
                g_buildRunning = true;
                g_lastBuildOk  = false;
                g_showGuide    = false;

                // 1. Gecici kaynak dosyasini kaydet (.cpp veya .c)
                const char* srcExt = (selectedLang == 0) ? ".cpp" : ".c";
                snprintf(g_tempCppPath, MAX_PATH, "%s\\_build_temp%s", PROJECTS_DIR, srcExt);
                snprintf(g_tempExePath, MAX_PATH, "%s\\_build_temp.exe", PROJECTS_DIR);

                FILE* fout = nullptr;
                fopen_s(&fout, g_tempCppPath, "w");
                if (fout) {
                    fwrite(currentCode.c_str(), 1, currentCode.size(), fout);
                    fclose(fout);

                    // 2. Gecici .bat dosyasina derle + calistir komutlarini yaz
                    static char g_tempBatPath[MAX_PATH];
                    snprintf(g_tempBatPath, MAX_PATH, "%s\\_build_run.bat", PROJECTS_DIR);

                    FILE* fbat = nullptr;
                    fopen_s(&fbat, g_tempBatPath, "w");
                    if (!fbat) {
                        g_buildOutput  = "[HATA] Gecici .bat dosyasi olusturulamadi!\n";
                        g_buildRunning = false;
                    } else {
                        fprintf(fbat, "@echo off\r\n");
                        fprintf(fbat, "call \"%s\" >nul 2>&1\r\n", VCVARS_PATH);
                        if (selectedLang == 0) {
                            fprintf(fbat, "cl.exe /nologo /std:c++17 /O1 /EHsc \"%s\" /Fe:\"%s\"\r\n", g_tempCppPath, g_tempExePath);
                        } else {
                            fprintf(fbat, "cl.exe /nologo /TC /O1 \"%s\" /Fe:\"%s\"\r\n", g_tempCppPath, g_tempExePath);
                        }
                        fclose(fbat);

                        std::string runBat = std::string("cmd.exe /c \"") + g_tempBatPath + "\" 2>&1";

                        g_buildOutput  = (selectedLang == 0)
                            ? (isEn ? "=== Compiling Modern C++ (cl.exe / C++17) ===\n" : "=== Modern C++ (cl.exe / C++17) Derleniyor ===\n")
                            : (isEn ? "=== Compiling ANSI C (cl.exe / C99) ===\n" : "=== Saf ANSI C (cl.exe / C99) Derleniyor ===\n");
                        g_buildOutput += "Kaynak: ";
                        g_buildOutput += g_tempCppPath;
                        g_buildOutput += "\n\n";
                        g_buildOutput += RunCommandCapture(runBat, g_lastBuildOk);

                        // 3. Derleme basarili ise calistir
                        if (g_lastBuildOk) {
                            g_buildOutput += isEn
                                ? "\n=== Build OK! Running program... ===\n"
                                : "\n=== Derleme Basarili! Program calistiriliyor... ===\n";

                            std::string runCmd = "\"" + std::string(g_tempExePath) + "\" 2>&1";
                            FILE* runPipe = _popen(runCmd.c_str(), "r");
                            if (runPipe) {
                                char buf[512];
                                int lineCount = 0;
                                while (fgets(buf, sizeof(buf), runPipe) && lineCount < 200) {
                                    g_buildOutput += buf;
                                    lineCount++;
                                }
                                if (lineCount >= 200) {
                                    g_buildOutput += isEn
                                        ? "\n[UYARI] 200 satir siniri asildi — sonsuz dongu ihtimali! Durduruldu.\n"
                                        : "\n[WARNING] 200 line limit reached — possible infinite loop! Stopped.\n";
                                }
                                _pclose(runPipe);
                            }
                        } else {
                            g_buildOutput += isEn
                                ? "\n[BUILD FAILED — Yukardaki derleyici hatalarini inceleyin.]\n"
                                : "\n[DERLEME BASARISIZ — Yukardaki derleyici hatalarini inceleyin.]\n";
                        }
                        g_buildRunning = false;
                    }
                } else {
                    g_buildOutput  = "[HATA] Gecici kaynak dosyasi olusturulamadi!\n";
                    g_buildRunning = false;
                }
            }
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            isEn ? "  Building..." : "  Derleniyor...");
    }

    ImGui::SameLine();

    // Harici CMD penceresi dugmesi
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.22f, 0.52f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.38f, 0.80f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.05f, 0.18f, 0.40f, 1.0f));
    if (ImGui::Button(isEn ? "Open in CMD" : "CMD'de Ac", ImVec2(105, 30))) {
        const char* srcExt = (selectedLang == 0) ? ".cpp" : ".c";
        snprintf(g_tempCppPath, MAX_PATH, "%s\\_build_temp%s", PROJECTS_DIR, srcExt);
        snprintf(g_tempExePath, MAX_PATH, "%s\\_build_temp.exe", PROJECTS_DIR);
        FILE* fout = nullptr;
        fopen_s(&fout, g_tempCppPath, "w");
        if (fout) { fwrite(currentCode.c_str(), 1, currentCode.size(), fout); fclose(fout); }

        char interactiveBatPath[MAX_PATH];
        snprintf(interactiveBatPath, MAX_PATH, "%s\\_run_interactive.bat", PROJECTS_DIR);
        FILE* fcmd = nullptr;
        fopen_s(&fcmd, interactiveBatPath, "w");
        if (fcmd) {
            fprintf(fcmd, "@echo off\r\n");
            fprintf(fcmd, "title C-Algorithm Interactive Runner (%s)\r\n", (selectedLang == 0) ? "C++17" : "ANSI C");
            fprintf(fcmd, "echo === C-Algorithm: Program Derleniyor ve Baslatiliyor (%s)... ===\r\n", (selectedLang == 0) ? "C++17" : "ANSI C");
            fprintf(fcmd, "call \"%s\" >nul 2>&1\r\n", VCVARS_PATH);
            if (selectedLang == 0) {
                fprintf(fcmd, "cl.exe /nologo /std:c++17 /O1 /EHsc \"%s\" /Fe:\"%s\"\r\n", g_tempCppPath, g_tempExePath);
            } else {
                fprintf(fcmd, "cl.exe /nologo /TC /O1 \"%s\" /Fe:\"%s\"\r\n", g_tempCppPath, g_tempExePath);
            }
            fprintf(fcmd, "if %%errorlevel%% equ 0 (\r\n");
            fprintf(fcmd, "    echo === PROGRAM CALISTIRILIYOR ===\r\n");
            fprintf(fcmd, "    echo.\r\n");
            fprintf(fcmd, "    \"%s\"\r\n", g_tempExePath);
            fprintf(fcmd, "    echo.\r\n");
            fprintf(fcmd, "    echo === Program sonlandi. Pencereyi kapatabilirsiniz. ===\r\n");
            fprintf(fcmd, ") else (\r\n");
            fprintf(fcmd, "    echo [HATA] Derleme basarisiz oldu! Yukaridaki derleyici ciktisini inceleyin.\r\n");
            fprintf(fcmd, ")\r\n");
            fclose(fcmd);
        }

        std::string extCmd = "start cmd.exe /k \"" + std::string(interactiveBatPath) + "\"";
        system(extCmd.c_str());
        pm.statusMessage = isEn ? "External CMD window opened!" : "Harici CMD penceresi acildi!";
    }

    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    // Gereksinimler rehberi toggle
    if (ImGui::Button(isEn ? "? Guide" : "? Rehber", ImVec2(75, 30))) {
        g_showGuide = !g_showGuide;
    }

    ImGui::SameLine();
    if (g_lastBuildOk) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f),
            isEn ? "  BUILD OK" : "  DERLEME OK");
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(); // CodegenBar ChildBg

    // ── Gereksinimler Rehberi (acik oldugunda) ───────────────────────────
    if (g_showGuide) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.08f, 0.04f, 1.0f));
        ImGui::BeginChild("GuidePanel", ImVec2(0, 175), true);
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f),
            isEn ? "COMPILER REQUIREMENTS GUIDE" : "DERLEYICI GEREKSINIMLERI REHBERI");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
            "[ONERILIR] Microsoft Visual Studio Build Tools 2022 (MSVC / cl.exe)");
        ImGui::BulletText("https://visualstudio.microsoft.com/tr/downloads/");
        ImGui::BulletText(isEn
            ? "Install: 'Desktop development with C++' workload"
            : "Yukleyin: 'C++ ile masaustu gelistirme' is yukunu secin");
        ImGui::BulletText("Beklenen yol: C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f),
            "[ALTERNATIF] MinGW-w64 / g++ (Windows icin GCC)");
        ImGui::BulletText("https://winlibs.com  veya  https://github.com/niXman/mingw-builds-binaries/releases");
        ImGui::BulletText(isEn
            ? "After install, add bin/ to System PATH."
            : "Kurulumdan sonra bin/ klasorunu Sistem PATH'ine ekleyin.");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "[ONLINE] Kurmadan deneyin: https://godbolt.org  |  https://onlinegdb.com");
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // ── Kod Gorunumu (ust kisim) ──────────────────────────────────────────
    float avail   = ImGui::GetContentRegionAvail().y;
    float codeH   = g_buildOutput.empty() ? avail : avail * 0.52f;
    float termH   = avail - codeH - 6.0f;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.03f, 0.04f, 0.06f, 1.0f));
    ImGui::BeginChild("CodegenView", ImVec2(0, codeH), true);
    ImGui::InputTextMultiline("##srcview", (char*)currentCode.c_str(), currentCode.size() + 1,
        ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
    ImGui::EndChild();
    ImGui::PopStyleColor();

    // ── Terminal / Build Output (alt kisim) ───────────────────────────────
    if (!g_buildOutput.empty() && termH > 10.0f) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.01f, 0.02f, 0.01f, 1.0f));
        ImGui::BeginChild("BuildTerminal", ImVec2(0, termH), true);

        ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.4f, 1.0f),
            isEn ? ">>> BUILD & RUN TERMINAL" : ">>> DERLEME & CALISTIRMA TERMINALI");
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 72);
        if (ImGui::SmallButton(isEn ? "Clear" : "Temizle")) {
            g_buildOutput.clear();
            g_lastBuildOk = false;
        }
        ImGui::Separator();

        // Satir satir renklendirme
        std::istringstream stream(g_buildOutput);
        std::string line;
        while (std::getline(stream, line)) {
            bool isError   = line.find("error")     != std::string::npos
                          || line.find("HATA")      != std::string::npos
                          || line.find("BASARISIZ") != std::string::npos
                          || line.find("FAILED")    != std::string::npos;
            bool isWarning = line.find("warning")   != std::string::npos
                          || line.find("UYARI")     != std::string::npos;
            bool isOk      = line.find("OK")        != std::string::npos
                          || line.find("Basarili")  != std::string::npos;
            bool isSys     = !line.empty() && line[0] == '=';

            if      (isError)   ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", line.c_str());
            else if (isWarning) ImGui::TextColored(ImVec4(1.0f, 0.80f, 0.20f, 1.0f), "%s", line.c_str());
            else if (isOk)      ImGui::TextColored(ImVec4(0.30f, 1.00f, 0.50f, 1.0f), "%s", line.c_str());
            else if (isSys)     ImGui::TextColored(ImVec4(0.00f, 0.85f, 1.00f, 1.0f), "%s", line.c_str());
            else                ImGui::TextUnformatted(line.c_str());
        }

        // Otomatik en alta kaydir
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 60.0f)
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
}

// ── 5. Inspector Panel ───────────────────────────────────────────────────
void RenderInspectorPanel(ProjectManager& pm, int& selectedNodeId) {
    bool isEn = (pm.currentLanguage == AppLanguage::English);

    ImGui::TextColored(ImVec4(0.05f, 0.58f, 0.53f, 1.0f), isEn ? "ALGORITHM SCHEMA PROPERTIES" : "Algoritma Şeması Özellikleri");
    ImGui::Separator();

    if (selectedNodeId > 0) {
        FlowNode* n = pm.GetNode(selectedNodeId);
        if (n) {
            ImGui::Text(isEn ? "Schema: #%d (%s)" : "Algoritma Şeması: #%d (%s)", n->id, n->title.c_str());

            if (n->kind == NodeKind::VariableDeclare) {
                char vName[64];
                strncpy(vName, n->targetVar.c_str(), sizeof(vName));
                if (ImGui::InputText(isEn ? "Variable Name" : "Değişken Adı", vName, sizeof(vName))) {
                    n->targetVar = vName;
                }

                ImGui::Checkbox(isEn ? "const (Read-Only / Immutable)" : "const (Değiştirilemez / Salt Okunur)", &n->isConst);
                if (n->isConst) {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), isEn ? "Immutable: Value cannot be altered after initialization." : "Değişmez: Tanımlandıktan sonra mutasyona uğratılamaz!");
                }

                // Özel Tip (Custom Struct) veya Standart C++ Tipi
                ImGui::Checkbox(isEn ? "Use Custom Struct Type" : "Özel Struct Tipinde Tanımla", &n->isCustomType);
                if (n->isCustomType) {
                    char customBuf[64];
                    strncpy(customBuf, n->customTypeName.c_str(), sizeof(customBuf));
                    if (ImGui::InputText(isEn ? "Struct Type Name" : "Struct Tip Adı (örn: Oyuncu)", customBuf, sizeof(customBuf))) {
                        n->customTypeName = customBuf;
                    }
                } else {
                    const char* typeNames[] = { "int (Int32)", "long long (Int64)", "float", "double", "bool", "uint8_t (0-255)", "uint16_t", "uint32_t", "uint64_t", "std::string" };
                    int currentType = (int)n->varType;
                    if (ImGui::Combo(isEn ? "Data Type" : "Veri Türü", &currentType, typeNames, IM_ARRAYSIZE(typeNames))) {
                        n->varType = (TypeKind)currentType;
                    }
                }

                // Akıllı İşaretçi / Bellek Yönetim Modu
                const char* ptrModes[] = { 
                    "Normal Değer (Stack)", 
                    "std::shared_ptr (Akıllı İşaretçi / RAII)", 
                    "std::unique_ptr (Tekil İşaretçi / Heap)" 
                };
                int currentPtrMode = (int)n->pointerKind;
                if (ImGui::Combo(isEn ? "Memory / Pointer Mode" : "Bellek & İşaretçi Modu", &currentPtrMode, ptrModes, IM_ARRAYSIZE(ptrModes))) {
                    n->pointerKind = (PointerKind)currentPtrMode;
                }

                if (n->pointerKind == PointerKind::SharedPtr) {
                    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.8f, 1.0f), 
                        isEn ? "Shared Ownership: Managed via std::make_shared with reference count." 
                             : "Akıllı Yönetim: std::make_shared ile heap tahsisi yapılır. Referans bittiğinde bellek oto-silinir!");
                } else if (n->pointerKind == PointerKind::UniquePtr) {
                    ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.2f, 1.0f), 
                        isEn ? "Exclusive Ownership: Exactly one owner on the heap." 
                             : "Tekil Sahiplik: Yalnızca tek sahip heap üzerinde yönetir.");
                }

                if (!n->isCustomType || n->pointerKind == PointerKind::RawValue) {
                    char valBuf[64];
                    strncpy(valBuf, n->expr.c_str(), sizeof(valBuf));
                    if (ImGui::InputText(isEn ? "Initial Value" : "Başlangıç Değeri", valBuf, sizeof(valBuf))) {
                        n->expr = valBuf;
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "Extra Variables in this Node:" : "Bu Düğümdeki Ek Değişkenler:");

                const char* typeNames[] = { "int (Int32)", "long long (Int64)", "float", "double", "bool", "uint8_t (0-255)", "uint16_t", "uint32_t", "uint64_t", "std::string" };
                for (size_t i = 0; i < n->multiAssignments.size(); i++) {
                    ImGui::PushID((int)i);
                    char extraName[64];
                    strncpy(extraName, n->multiAssignments[i].varName.c_str(), sizeof(extraName));
                    if (ImGui::InputText(isEn ? "Name" : "Ad", extraName, sizeof(extraName))) {
                        n->multiAssignments[i].varName = extraName;
                    }
                    int eType = (int)n->multiAssignments[i].varType;
                    if (ImGui::Combo(isEn ? "Type" : "Tür", &eType, typeNames, IM_ARRAYSIZE(typeNames))) {
                        n->multiAssignments[i].varType = (TypeKind)eType;
                    }
                    char extraVal[64];
                    strncpy(extraVal, n->multiAssignments[i].valueExpr.c_str(), sizeof(extraVal));
                    if (ImGui::InputText(isEn ? "Value" : "Değer", extraVal, sizeof(extraVal))) {
                        n->multiAssignments[i].valueExpr = extraVal;
                    }
                    if (ImGui::Button(isEn ? "Remove Variable" : "Değişkeni Kaldır")) {
                        n->multiAssignments.erase(n->multiAssignments.begin() + i);
                        ImGui::PopID();
                        break;
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                }

                if (ImGui::Button(isEn ? "+ Add Extra Variable" : "+ Ek Değişken Ekle")) {
                    n->multiAssignments.push_back({ "var_" + std::to_string(n->multiAssignments.size() + 1), TypeKind::Int32, "0" });
                }
            } else if (n->kind == NodeKind::Print) {
                const char* printModes[] = {
                    isEn ? "Plain Text (std::cout << \"...\")" : "Düz Metin (std::cout << \"...\")",
                    isEn ? "Variable Value (std::cout << x)" : "Değişken Değeri (std::cout << x)",
                    isEn ? "Text + Variable (std::cout << \"...\" << x)" : "Metin + Değişken (std::cout << \"...\" << x)"
                };
                ImGui::Combo(isEn ? "Print Mode" : "Yazdırma Modu", &n->printMode, printModes, IM_ARRAYSIZE(printModes));

                if (n->printMode == 0 || n->printMode == 2) {
                    char textBuf[128];
                    strncpy(textBuf, n->ioText.c_str(), sizeof(textBuf));
                    if (ImGui::InputText(isEn ? "Text to Print" : "Yazdırılacak Metin", textBuf, sizeof(textBuf))) {
                        n->ioText = textBuf;
                    }
                }
                if (n->printMode == 1 || n->printMode == 2) {
                    char varBuf[64];
                    strncpy(varBuf, n->targetVar.c_str(), sizeof(varBuf));
                    if (ImGui::InputText(isEn ? "Variable Name" : "Yazdırılacak Değişken", varBuf, sizeof(varBuf))) {
                        n->targetVar = varBuf;
                    }
                }

                // Canli Kod Onizlemesi
                if (n->printMode == 1) {
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "  -> std::cout << %s << std::endl;", n->targetVar.c_str());
                } else if (n->printMode == 2) {
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "  -> std::cout << \"%s \" << %s << std::endl;", n->ioText.c_str(), n->targetVar.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "  -> std::cout << \"%s\" << std::endl;", n->ioText.c_str());
                }

            } else if (n->kind == NodeKind::Input) {
                char varBuf[64];
                strncpy(varBuf, n->targetVar.c_str(), sizeof(varBuf));
                if (ImGui::InputText(isEn ? "Target Variable to Read" : "Girdinin Yazılacağı Değişken", varBuf, sizeof(varBuf))) {
                    n->targetVar = varBuf;
                }
                if (n->targetVar.empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                        isEn ? "  [!] Specify a variable name (e.g. isim, yas)"
                             : "  [!] Bir değişken adı yazın (ör: isim, yas)");
                } else {
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
                        isEn ? "  -> Generates: std::cin >> %s;"
                             : "  -> Üretilen: std::cin >> %s;", n->targetVar.c_str());
                }

            } else if (n->kind == NodeKind::Assign) {
                char varBuf[64];
                strncpy(varBuf, n->targetVar.c_str(), sizeof(varBuf));
                if (ImGui::InputText(isEn ? "Target Variable" : "Hedef Değişken", varBuf, sizeof(varBuf))) {
                    n->targetVar = varBuf;
                }
                char exprBuf[64];
                strncpy(exprBuf, n->expr.c_str(), sizeof(exprBuf));
                if (ImGui::InputText(isEn ? "Value / Expression" : "Atanacak Değer / İfade", exprBuf, sizeof(exprBuf))) {
                    n->expr = exprBuf;
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "Extra Assignments in this Node:" : "Bu Düğümdeki Ek Atamalar:");

                for (size_t i = 0; i < n->multiAssignments.size(); i++) {
                    ImGui::PushID((int)(100 + i));
                    char extraName[64];
                    strncpy(extraName, n->multiAssignments[i].varName.c_str(), sizeof(extraName));
                    if (ImGui::InputText(isEn ? "Variable" : "Değişken", extraName, sizeof(extraName))) {
                        n->multiAssignments[i].varName = extraName;
                    }
                    char extraVal[64];
                    strncpy(extraVal, n->multiAssignments[i].valueExpr.c_str(), sizeof(extraVal));
                    if (ImGui::InputText(isEn ? "New Value" : "Yeni Değer", extraVal, sizeof(extraVal))) {
                        n->multiAssignments[i].valueExpr = extraVal;
                    }
                    if (ImGui::Button(isEn ? "Remove Assignment" : "Atamayı Kaldır")) {
                        n->multiAssignments.erase(n->multiAssignments.begin() + i);
                        ImGui::PopID();
                        break;
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                }

                if (ImGui::Button(isEn ? "+ Add Extra Assignment" : "+ Ek Atama Ekle")) {
                    n->multiAssignments.push_back({ "x", TypeKind::Int32, "0" });
                }
            } else if (n->kind == NodeKind::MathOp) {
                ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "Math Calculation Setup:" : "Matematiksel Hesaplama Ayarı:");

                char targetBuf[64];
                strncpy(targetBuf, n->mathTarget.c_str(), sizeof(targetBuf));
                if (ImGui::InputText(isEn ? "Result Variable" : "Sonucun Yazılacağı Değişken", targetBuf, sizeof(targetBuf))) {
                    n->mathTarget = targetBuf;
                }

                char leftBuf[64];
                strncpy(leftBuf, n->mathLeft.c_str(), sizeof(leftBuf));
                if (ImGui::InputText(isEn ? "1st Value / Variable" : "1. Terim / Değişken", leftBuf, sizeof(leftBuf))) {
                    n->mathLeft = leftBuf;
                }

                const char* ops[] = {
                    "+ (Toplama / Add)",
                    "- (Cikarma / Subtract)",
                    "* (Carpma / Multiply)",
                    "/ (Bolme / Divide)",
                    "% (Mod Alma / Modulo)",
                    "sqrt (Karekok / Square Root)",
                    "pow (Us Alma / Power)",
                    "fact (Faktoriyel / Factorial n!)"
                };
                const char* opTokens[] = { "+", "-", "*", "/", "%", "sqrt", "pow", "fact" };

                int currentOpIdx = 0;
                for (int i = 0; i < IM_ARRAYSIZE(opTokens); ++i) {
                    if (n->mathOp == opTokens[i]) { currentOpIdx = i; break; }
                }

                if (ImGui::Combo(isEn ? "Operation" : "İşlem Türü", &currentOpIdx, ops, IM_ARRAYSIZE(ops))) {
                    n->mathOp = opTokens[currentOpIdx];
                    if (n->mathOp == "fact") n->title = isEn ? "Factorial (!)" : "Faktöriyel (!)";
                    else if (n->mathOp == "sqrt") n->title = isEn ? "Square Root (sqrt)" : "Karekök (sqrt)";
                    else n->title = "Math (" + n->mathOp + ")";
                }

                bool isUnary = (n->mathOp == "sqrt" || n->mathOp == "fact");
                if (!isUnary) {
                    char rightBuf[64];
                    strncpy(rightBuf, n->mathRight.c_str(), sizeof(rightBuf));
                    if (ImGui::InputText(isEn ? "2nd Value / Variable" : "2. Terim / Değişken", rightBuf, sizeof(rightBuf))) {
                        n->mathRight = rightBuf;
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                        isEn ? "Single-operand operation (only 1st value is used)."
                             : "Tek terimli işlem (yalnızca 1. terim kullanılır).");
                }

                // Canlı Kod Önizlemesi
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), isEn ? "Generated C++ Code Preview:" : "Üretilen C++ Önizlemesi:");
                if (n->mathOp == "sqrt") {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "  %s = std::sqrt(%s);", n->mathTarget.c_str(), n->mathLeft.c_str());
                } else if (n->mathOp == "pow") {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "  %s = std::pow(%s, %s);", n->mathTarget.c_str(), n->mathLeft.c_str(), n->mathRight.c_str());
                } else if (n->mathOp == "fact") {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "  // Faktoriyel for dongusu (%s!)", n->mathLeft.c_str());
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "  %s = 1; for(int i=1; i<=%s; ++i) %s *= i;", n->mathTarget.c_str(), n->mathLeft.c_str(), n->mathTarget.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "  %s = %s %s %s;", n->mathTarget.c_str(), n->mathLeft.c_str(), n->mathOp.c_str(), n->mathRight.c_str());
                }
            } else if (n->kind == NodeKind::LoopWhile || n->kind == NodeKind::LoopDoWhile) {
                bool isDoWhile = (n->kind == NodeKind::LoopDoWhile);

                // ── Koşul alanı ─────────────────────────────────────────
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f),
                    isEn ? "1. Loop Condition:" : "1. Döngü Koşulu:");
                char condBuf[128];
                strncpy(condBuf, n->conditionExpr.c_str(), sizeof(condBuf));
                if (ImGui::InputText(isEn ? "Condition" : "Koşul", condBuf, sizeof(condBuf))) {
                    n->conditionExpr = condBuf;
                }
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                    isEn ? "Example: sayac < 10, hp > 0" : "Örnek: sayac < 10, hp > 0");

                ImGui::Spacing();
                ImGui::Separator();

                // ── Gövde ifadeleri (sayac++, x *= 2, vb.) ──────────────
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f),
                    isEn ? "2. Body Statements (inside { }):" : "2. Gövde İfadeleri ({ } içi):");
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                    isEn ? "These run every loop iteration. e.g: sayac++, x = x + 1"
                         : "Her turda çalışır. Örnek: sayac++, x = x + 1, toplam += i");

                ImGui::Spacing();
                for (int si = 0; si < (int)n->loopBodyStmts.size(); si++) {
                    ImGui::PushID(si + 8000);
                    char stmtBuf[128];
                    strncpy(stmtBuf, n->loopBodyStmts[si].c_str(), sizeof(stmtBuf));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 64);
                    if (ImGui::InputText("##bodyStmt", stmtBuf, sizeof(stmtBuf))) {
                        n->loopBodyStmts[si] = stmtBuf;
                    }

                    // ── Anlık Uyarı Kontrolü ────────────────────────────
                    {
                        const std::string& s = n->loopBodyStmts[si];
                        // Bare 'endl' — std::endl olmadan
                        bool badEndl  = s.find("endl")     != std::string::npos
                                     && s.find("std::endl") == std::string::npos;
                        // Bare 'cout'  — std::cout olmadan
                        bool badCout  = s.find("cout")     != std::string::npos
                                     && s.find("std::cout") == std::string::npos;
                        // Bare 'cin'   — std::cin olmadan
                        bool badCin   = s.find("cin")      != std::string::npos
                                     && s.find("std::cin")  == std::string::npos;
                        // Sona ; koymuş — codegen zaten ekliyor, çift ; hata verir
                        bool trailSemi = !s.empty() && s.back() == ';';

                        if (badEndl)
                            ImGui::TextColored(ImVec4(1.f,0.3f,0.3f,1.f),
                                isEn ? "  [!] Use std::endl not endl"
                                     : "  [!] 'endl' yerine 'std::endl' yazin");
                        if (badCout)
                            ImGui::TextColored(ImVec4(1.f,0.3f,0.3f,1.f),
                                isEn ? "  [!] Use std::cout not cout"
                                     : "  [!] 'cout' yerine 'std::cout' yazin");
                        if (badCin)
                            ImGui::TextColored(ImVec4(1.f,0.3f,0.3f,1.f),
                                isEn ? "  [!] Use std::cin not cin"
                                     : "  [!] 'cin' yerine 'std::cin' yazin");
                        if (trailSemi)
                            ImGui::TextColored(ImVec4(1.f,0.7f,0.1f,1.f),
                                isEn ? "  [!] Remove trailing ';' — codegen adds it"
                                     : "  [!] Sona ';' koyma — codegen otomatik ekliyor");
                    }

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.15f, 0.15f, 1.0f));
                    if (ImGui::SmallButton("X")) {
                        n->loopBodyStmts.erase(n->loopBodyStmts.begin() + si);
                        ImGui::PopStyleColor();
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                }

                if (ImGui::Button(isEn ? "+ Add Body Statement" : "+ Gövde İfadesi Ekle",
                                  ImVec2(-1, 28))) {
                    n->loopBodyStmts.push_back(isDoWhile ? "sayac++" : "sayac++");
                }

                ImGui::Spacing();
                // Özet önizleme
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.02f, 0.03f, 0.04f, 1.0f));
                ImGui::BeginChild("LoopPreview", ImVec2(0, 72), true);
                if (isDoWhile) {
                    ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), "do {");
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f),
                        "while (%s) {", n->conditionExpr.c_str());
                }
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "    // ... bagli dugumler ...");
                for (const auto& s : n->loopBodyStmts)
                    if (!s.empty())
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "    %s;", s.c_str());
                if (isDoWhile)
                    ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f),
                        "} while (%s);", n->conditionExpr.c_str());
                else
                    ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), "}");
                ImGui::EndChild();
                ImGui::PopStyleColor();
            } else if (n->kind == NodeKind::BranchIf) {
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f),
                    isEn ? "If / Else Condition:" : "Koşul (If / Else):");
                char condBuf[128];
                strncpy(condBuf, n->conditionExpr.c_str(), sizeof(condBuf));
                if (ImGui::InputText(isEn ? "Condition" : "Mantıksal Koşul", condBuf, sizeof(condBuf))) {
                    n->conditionExpr = condBuf;
                }
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                    isEn ? "Example: x < 10, hp <= 0, puan >= 100" : "Örnek: x < 10, can <= 0, puan >= 100");
            } else if (n->kind == NodeKind::LoopFor) {
                char initBuf[64];
                strncpy(initBuf, n->loopInit.c_str(), sizeof(initBuf));
                if (ImGui::InputText(isEn ? "Init" : "Başlangıç (Init)", initBuf, sizeof(initBuf))) {
                    n->loopInit = initBuf;
                }
                char condBuf[128];
                strncpy(condBuf, n->conditionExpr.c_str(), sizeof(condBuf));
                if (ImGui::InputText(isEn ? "Condition" : "Koşul (Condition)", condBuf, sizeof(condBuf))) {
                    n->conditionExpr = condBuf;
                }
                char stepBuf[64];
                strncpy(stepBuf, n->loopStep.c_str(), sizeof(stepBuf));
                if (ImGui::InputText(isEn ? "Step" : "Artış (Step)", stepBuf, sizeof(stepBuf))) {
                    n->loopStep = stepBuf;
                }
            } else if (n->kind == NodeKind::LoopEnd) {
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), isEn ? "Loop End Schema (}):" : "Döngü Sonu Şeması (}):");
                ImGui::TextWrapped(isEn 
                    ? "Marks the boundary of the loop body. All schemas executed repeatedly must terminate into this node. The right output pin continues the program flow outside the loop." 
                    : "Döngü gövdesinin bittiği sınırı işaretler ('}' karakteri). Döngü içinde tekrarlanacak tüm şemalar bu şemaya bağlanır; bu şemanın sağ çıkış pini ise döngü dışındaki akıştan devam eder.");
            } else if (n->kind == NodeKind::SwitchCase) {
                char swBuf[64];
                strncpy(swBuf, n->switchVar.c_str(), sizeof(swBuf));
                if (ImGui::InputText(isEn ? "Switch Variable" : "Değerlendirilecek Değişken", swBuf, sizeof(swBuf))) {
                    n->switchVar = swBuf;
                }
                ImGui::Spacing();
                ImGui::Text(isEn ? "Cases (Branches):" : "Durumlar (Case Dalları):");
                if (n->caseValues.size() > 0) {
                    char c1Buf[32];
                    strncpy(c1Buf, n->caseValues[0].c_str(), sizeof(c1Buf));
                    if (ImGui::InputText("Case 1 Değeri", c1Buf, sizeof(c1Buf))) {
                        n->caseValues[0] = c1Buf;
                    }
                }
                if (n->caseValues.size() > 1) {
                    char c2Buf[32];
                    strncpy(c2Buf, n->caseValues[1].c_str(), sizeof(c2Buf));
                    if (ImGui::InputText("Case 2 Değeri", c2Buf, sizeof(c2Buf))) {
                        n->caseValues[1] = c2Buf;
                    }
                }
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), isEn ? "Default and Completed pins are built-in." : "Default ve Bitti pinleri entegredir.");
            } else if (n->kind == NodeKind::StructDef) {
                char sName[64];
                strncpy(sName, n->structName.c_str(), sizeof(sName));
                if (ImGui::InputText(isEn ? "Struct Name" : "Struct Yapı Adı", sName, sizeof(sName))) {
                    n->structName = sName;
                    n->title = isEn ? ("Struct: " + n->structName) : ("Struct: " + n->structName);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "Struct Members (Fields):" : "Struct Alanları (Üye Değişkenler):");

                const char* typeNames[] = { "int (Int32)", "long long (Int64)", "float", "double", "bool", "uint8_t (0-255)", "uint16_t", "uint32_t", "uint64_t", "std::string" };

                for (size_t i = 0; i < n->structMembers.size(); i++) {
                    ImGui::PushID((int)(200 + i));
                    char mName[64];
                    strncpy(mName, n->structMembers[i].name.c_str(), sizeof(mName));
                    if (ImGui::InputText(isEn ? "Field Name" : "Alan Adı", mName, sizeof(mName))) {
                        n->structMembers[i].name = mName;
                    }
                    int mType = (int)n->structMembers[i].type;
                    if (ImGui::Combo(isEn ? "Field Type" : "Alan Türü", &mType, typeNames, IM_ARRAYSIZE(typeNames))) {
                        n->structMembers[i].type = (TypeKind)mType;
                    }

                    char defValBuf[64];
                    strncpy(defValBuf, n->structMembers[i].defaultValue.c_str(), sizeof(defValBuf));
                    if (ImGui::InputText(isEn ? "Default Value" : "Varsayılan Değer", defValBuf, sizeof(defValBuf))) {
                        n->structMembers[i].defaultValue = defValBuf;
                        n->structMembers[i].hasDefault = true;
                    }

                    if (ImGui::Button(isEn ? "Remove Field" : "Alanı Kaldır")) {
                        n->structMembers.erase(n->structMembers.begin() + i);
                        ImGui::PopID();
                        break;
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                }

                if (ImGui::Button(isEn ? "+ Add Field" : "+ Yeni Alan Ekle")) {
                    n->structMembers.push_back({ "alan_" + std::to_string(n->structMembers.size() + 1), TypeKind::Int32, "0", true });
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Checkbox(isEn ? "Generate Destructor (~Struct)" : "Yıkıcı Metot (~Destructor) Tanımla", &n->hasDestructor);
                if (n->hasDestructor) {
                    char dMsgBuf[128];
                    strncpy(dMsgBuf, n->destructorMsg.c_str(), sizeof(dMsgBuf));
                    if (ImGui::InputText(isEn ? "Destructor Log Message" : "Yıkıcı Konsol Mesajı", dMsgBuf, sizeof(dMsgBuf))) {
                        n->destructorMsg = dMsgBuf;
                    }
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), isEn 
                        ? "RAII: Automatically executed when the object is destroyed/freed." 
                        : "RAII: Nesne bellekten silindiğinde otomatik çağrılır (Unreal Destroy muadili).");
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), isEn ? "C++ Struct Preview:" : "C++ Struct Önizlemesi:");
                std::string preview = "struct " + n->structName + " {\n";
                for (const auto& m : n->structMembers) {
                    preview += "    " + std::string(TypeKindToString(m.type)) + " " + m.name;
                    if (m.hasDefault && !m.defaultValue.empty()) {
                        preview += " = " + m.defaultValue;
                    }
                    preview += ";\n";
                }
                if (n->hasDestructor) {
                    preview += "\n    // RAII Yikici Metot\n";
                    preview += "    ~" + n->structName + "() {\n";
                    preview += "        std::cout << \"[DESTRUCTOR] " + n->destructorMsg + "\" << std::endl;\n";
                    preview += "    }\n";
                }
                preview += "};";
                ImGui::TextUnformatted(preview.c_str());
            } else if (n->kind == NodeKind::VectorOp) {
                char vName[64];
                std::string currentDS = !n->dsName.empty() ? n->dsName : n->vectorName;
                strncpy(vName, currentDS.c_str(), sizeof(vName));
                if (ImGui::InputText(isEn ? "Vector Name" : "Vector Adı", vName, sizeof(vName))) {
                    n->dsName = vName;
                    n->vectorName = vName;
                }

                const char* opTypes[] = { "push_back (Sona Ekle)", "pop_back (Sondan Sil)", "clear (Tümünü Temizle)" };
                std::string currentOpStr = !n->dsOpType.empty() ? n->dsOpType : n->vectorOpType;
                int currentOp = (currentOpStr == "push_back") ? 0 : (currentOpStr == "pop_back" ? 1 : 2);
                if (ImGui::Combo(isEn ? "Operation" : "Vektör İşlemi", &currentOp, opTypes, IM_ARRAYSIZE(opTypes))) {
                    if (currentOp == 0) { n->dsOpType = "push_back"; n->vectorOpType = "push_back"; }
                    else if (currentOp == 1) { n->dsOpType = "pop_back"; n->vectorOpType = "pop_back"; }
                    else { n->dsOpType = "clear"; n->vectorOpType = "clear"; }
                }

                if (currentOp == 0) {
                    char argBuf[64];
                    std::string currentArg = !n->dsOpArg.empty() ? n->dsOpArg : n->vectorOpArg;
                    strncpy(argBuf, currentArg.c_str(), sizeof(argBuf));
                    if (ImGui::InputText(isEn ? "Value to Push" : "Eklenecek Değer / İfade", argBuf, sizeof(argBuf))) {
                        n->dsOpArg = argBuf;
                        n->vectorOpArg = argBuf;
                    }
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), isEn ? "Data Structure Note:" : "Veri Yapısı Notu:");
                ImGui::TextWrapped(isEn 
                    ? "std::vector is a contiguous dynamic array. push_back is O(1) amortized and CPU cache-friendly!" 
                    : "std::vector ardışık dinamik dizidir. push_back() CPU L1 önbellek dostudur ve O(1) sabit sürede çalışır!");
            } else if (n->kind == NodeKind::ListOp) {
                char lName[64];
                strncpy(lName, n->dsName.c_str(), sizeof(lName));
                if (ImGui::InputText(isEn ? "List Name" : "Bağlı Liste Adı", lName, sizeof(lName))) {
                    n->dsName = lName;
                }

                const char* opTypes[] = { "push_back (Sona Ekle)", "push_front (Başa Ekle)", "pop_front (Baştan Sil)", "pop_back (Sondan Sil)", "clear (Temizle)" };
                int currentOp = (n->dsOpType == "push_back") ? 0 :
                                (n->dsOpType == "push_front") ? 1 :
                                (n->dsOpType == "pop_front") ? 2 :
                                (n->dsOpType == "pop_back") ? 3 : 4;
                if (ImGui::Combo(isEn ? "Operation" : "Liste İşlemi", &currentOp, opTypes, IM_ARRAYSIZE(opTypes))) {
                    if (currentOp == 0) n->dsOpType = "push_back";
                    else if (currentOp == 1) n->dsOpType = "push_front";
                    else if (currentOp == 2) n->dsOpType = "pop_front";
                    else if (currentOp == 3) n->dsOpType = "pop_back";
                    else n->dsOpType = "clear";
                }

                if (currentOp == 0 || currentOp == 1) {
                    char argBuf[64];
                    strncpy(argBuf, n->dsOpArg.c_str(), sizeof(argBuf));
                    if (ImGui::InputText(isEn ? "Value to Insert" : "Eklenecek Değer", argBuf, sizeof(argBuf))) {
                        n->dsOpArg = argBuf;
                    }
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.7f, 1.0f), isEn ? "Data Structure Note:" : "Veri Yapısı Notu:");
                ImGui::TextWrapped(isEn 
                    ? "std::list is a doubly linked list. Inserting/removing at front or back is strictly O(1) time with pointer rewiring." 
                    : "std::list çift yönlü bağlı listedir (Doubly Linked List). Başa ve sona ekleme/silme kesin olarak O(1) sürededir!");
            } else if (n->kind == NodeKind::StackOp) {
                char sName[64];
                strncpy(sName, n->dsName.c_str(), sizeof(sName));
                if (ImGui::InputText(isEn ? "Stack Name" : "Yığın (Stack) Adı", sName, sizeof(sName))) {
                    n->dsName = sName;
                }

                const char* opTypes[] = { "push (Tepeye Ekle)", "pop (Tepeden Çıkar)", "top (Tepe Elemanı Oku)" };
                int currentOp = (n->dsOpType == "push") ? 0 : (n->dsOpType == "pop" ? 1 : 2);
                if (ImGui::Combo(isEn ? "Operation" : "Yığın İşlemi", &currentOp, opTypes, IM_ARRAYSIZE(opTypes))) {
                    if (currentOp == 0) n->dsOpType = "push";
                    else if (currentOp == 1) n->dsOpType = "pop";
                    else n->dsOpType = "top";
                }

                if (currentOp == 0) {
                    char argBuf[64];
                    strncpy(argBuf, n->dsOpArg.c_str(), sizeof(argBuf));
                    if (ImGui::InputText(isEn ? "Value to Push" : "Tepeye Eklenecek Değer", argBuf, sizeof(argBuf))) {
                        n->dsOpArg = argBuf;
                    }
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.9f, 1.0f), isEn ? "Data Structure Note:" : "Veri Yapısı Notu:");
                ImGui::TextWrapped(isEn 
                    ? "std::stack operates on LIFO (Last In, First Out). Used in function call stacks, recursion, and Undo systems!" 
                    : "std::stack Son Giren İlk Çıkar (LIFO) prensibiyle çalışır. Fonksiyon çağrı yığını, özyineleme ve Geri Al (Undo) mekanizmalarının temelidir!");
            } else if (n->kind == NodeKind::QueueOp) {
                char qName[64];
                strncpy(qName, n->dsName.c_str(), sizeof(qName));
                if (ImGui::InputText(isEn ? "Queue Name" : "Kuyruk (Queue) Adı", qName, sizeof(qName))) {
                    n->dsName = qName;
                }

                const char* opTypes[] = { "push (Kuyruğa Ekle)", "pop (Kuyruktan Çıkar)", "front (Öndekini Oku)" };
                int currentOp = (n->dsOpType == "push") ? 0 : (n->dsOpType == "pop" ? 1 : 2);
                if (ImGui::Combo(isEn ? "Operation" : "Kuyruk İşlemi", &currentOp, opTypes, IM_ARRAYSIZE(opTypes))) {
                    if (currentOp == 0) n->dsOpType = "push";
                    else if (currentOp == 1) n->dsOpType = "pop";
                    else n->dsOpType = "front";
                }

                if (currentOp == 0) {
                    char argBuf[64];
                    strncpy(argBuf, n->dsOpArg.c_str(), sizeof(argBuf));
                    if (ImGui::InputText(isEn ? "Value to Enqueue" : "Kuyruğa Eklenecek Değer", argBuf, sizeof(argBuf))) {
                        n->dsOpArg = argBuf;
                    }
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), isEn ? "Data Structure Note:" : "Veri Yapısı Notu:");
                ImGui::TextWrapped(isEn 
                    ? "std::queue operates on FIFO (First In, First Out). Essential for task schedulers, print spoolers, and message queues!" 
                    : "std::queue İlk Giren İlk Çıkar (FIFO) prensibiyle çalışır. Görev sıralayıcılar ve mesaj kuyruklarının temelidir!");
            } else if (n->kind == NodeKind::ArrayOp) {
                char aName[64];
                strncpy(aName, n->dsName.c_str(), sizeof(aName));
                if (ImGui::InputText(isEn ? "Array Name" : "Dizi Adı", aName, sizeof(aName))) {
                    n->dsName = aName;
                }

                ImGui::InputInt(isEn ? "Array Size (Fixed)" : "Dizi Boyutu (Sabit N)", &n->arraySize);
                if (n->arraySize < 1) n->arraySize = 1;

                ImGui::InputInt(isEn ? "Target Index" : "Erişilecek İndeks (0..N-1)", &n->arrayIndex);
                if (n->arrayIndex < 0) n->arrayIndex = 0;
                if (n->arrayIndex >= n->arraySize) n->arrayIndex = n->arraySize - 1;

                char argBuf[64];
                strncpy(argBuf, n->dsOpArg.c_str(), sizeof(argBuf));
                if (ImGui::InputText(isEn ? "Value at Index" : "İndekse Atanacak Değer", argBuf, sizeof(argBuf))) {
                    n->dsOpArg = argBuf;
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), isEn ? "Data Structure Note:" : "Veri Yapısı Notu:");
                ImGui::TextWrapped(isEn 
                    ? "Fixed-size arrays allocate contiguous memory on the stack with zero dynamic overhead. Direct indexing is O(1)!" 
                    : "Sabit boyutlu diziler Stack üzerinde ardışık ayrılır, sıfır dinamik yük taşır. İndeksle doğrudan erişim O(1) sabit süredir!");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // DUGUM SIL BUTONU
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.20f, 0.20f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.30f, 0.30f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.15f, 0.15f, 1.0f));
            if (ImGui::Button(isEn ? "Delete Schema" : "Algoritma Şemasını Sil", ImVec2(-1, 32))) {
                pm.DeleteNode(selectedNodeId);
                selectedNodeId = 0;
            }
            ImGui::PopStyleColor(3);
        }
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), isEn ? "Click on an algorithm schema in the graph to inspect." : "Düzenlemek için akıştan bir Algoritma Şemasına tıklayın veya çift tıklayın.");
    }
}

// ── 6. Tema ve Stil Yonetimi ─────────────────────────────────────────────
void ApplyAppTheme(AppTheme theme) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding    = 8.0f;
    style.FrameRounding     = 6.0f;
    style.PopupRounding     = 8.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding      = 5.0f;
    style.TabRounding       = 6.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;

    if (theme == AppTheme::LightPastelNeon) {
        ImGui::StyleColorsLight();
        
        // Zeminler: Fildisi ve Berrak Arduvaz
        colors[ImGuiCol_WindowBg]             = ImVec4(0.96f, 0.97f, 0.99f, 1.00f);
        colors[ImGuiCol_ChildBg]              = ImVec4(0.92f, 0.95f, 0.98f, 1.00f);
        colors[ImGuiCol_PopupBg]              = ImVec4(0.99f, 0.99f, 1.00f, 0.98f);
        colors[ImGuiCol_Border]               = ImVec4(0.05f, 0.58f, 0.53f, 0.55f); // Soft Petrol Yesili Işıma
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        
        // Metinler: Koyu Arduvaz (Maksimum Netlik & Kontrast)
        colors[ImGuiCol_Text]                 = ImVec4(0.06f, 0.09f, 0.16f, 1.00f);
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.45f, 0.52f, 0.60f, 1.00f);
        
        // Form & Girdi Alanlari
        colors[ImGuiCol_FrameBg]              = ImVec4(0.88f, 0.92f, 0.95f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.82f, 0.88f, 0.92f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.76f, 0.84f, 0.90f, 1.00f);
        
        // Basliklar (Elektrik Petrol Yesili / Deep Teal #0d9488)
        colors[ImGuiCol_TitleBg]              = ImVec4(0.05f, 0.58f, 0.53f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.07f, 0.68f, 0.62f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.05f, 0.58f, 0.53f, 0.75f);
        colors[ImGuiCol_MenuBarBg]            = ImVec4(0.92f, 0.94f, 0.96f, 1.00f);
        
        // Header
        colors[ImGuiCol_Header]               = ImVec4(0.05f, 0.58f, 0.53f, 0.25f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.96f, 0.62f, 0.04f, 0.40f); // Altın Sarısı Hover Vurgusu
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.96f, 0.25f, 0.37f, 0.60f); // Mercan Pembe Aktif
        
        // Butonlar (Elektrik Petrol Yeşili + Canlı Sarı/Pembe Hover & Vurgular)
        colors[ImGuiCol_Button]               = ImVec4(0.05f, 0.58f, 0.53f, 0.90f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.96f, 0.62f, 0.04f, 1.00f); // Parlak Altın Sarısı
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.96f, 0.25f, 0.37f, 1.00f); // Mercan Pembe
        
        // Sekmeler (Tabs): Parlak Sarı Hover ve Mercan Pembe Aktif
        colors[ImGuiCol_Tab]                  = ImVec4(0.86f, 0.91f, 0.95f, 1.00f);
        colors[ImGuiCol_TabHovered]           = ImVec4(0.96f, 0.62f, 0.04f, 0.85f); // Parlak Altın Sarısı
        colors[ImGuiCol_TabActive]            = ImVec4(0.96f, 0.25f, 0.37f, 1.00f); // Mercan Pembe Neon (#f43f5e)
        colors[ImGuiCol_TabUnfocused]         = ImVec4(0.88f, 0.91f, 0.94f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]  = ImVec4(0.05f, 0.58f, 0.53f, 0.75f);
        
        // Kontroller
        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.92f, 0.94f, 0.96f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.70f, 0.76f, 0.82f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.55f, 0.62f, 0.70f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.40f, 0.48f, 0.56f, 1.00f);
        colors[ImGuiCol_CheckMark]            = ImVec4(0.96f, 0.25f, 0.37f, 1.00f); // Mercan Pembe
        colors[ImGuiCol_SliderGrab]           = ImVec4(0.05f, 0.58f, 0.53f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.96f, 0.62f, 0.04f, 1.00f); // Parlak Altın Sarısı
        colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.08f, 0.12f, 0.18f, 0.45f);
    } else { // DarkCyberNeon — "Amethyst Night" (Neon Mor + Petrol Yeşili + Altın Sarısı)
        ImGui::StyleColorsDark();

        // Zeminler: Derin Neon Mor (Amethyst Night)
        colors[ImGuiCol_WindowBg]             = ImVec4(0.10f, 0.05f, 0.18f, 1.00f); // #1A0D2E derin mor
        colors[ImGuiCol_ChildBg]              = ImVec4(0.13f, 0.07f, 0.22f, 1.00f); // biraz daha açık ametist
        colors[ImGuiCol_PopupBg]              = ImVec4(0.14f, 0.07f, 0.24f, 0.97f);
        colors[ImGuiCol_Border]               = ImVec4(0.05f, 0.75f, 0.65f, 0.55f); // Petrol yeşili neon kenarlık

        colors[ImGuiCol_Text]                 = ImVec4(0.96f, 0.94f, 1.00f, 1.00f); // Lavantamsı beyaz
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.55f, 0.45f, 0.70f, 1.00f);

        // Giriş Alanları
        colors[ImGuiCol_FrameBg]              = ImVec4(0.18f, 0.09f, 0.30f, 1.00f); // Mor kadife
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.24f, 0.12f, 0.38f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.30f, 0.15f, 0.46f, 1.00f);

        // Başlık Çubukları
        colors[ImGuiCol_TitleBg]              = ImVec4(0.14f, 0.06f, 0.24f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.05f, 0.65f, 0.56f, 0.90f); // Petrol yeşili aktif başlık
        colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.10f, 0.05f, 0.18f, 0.75f);
        colors[ImGuiCol_MenuBarBg]            = ImVec4(0.12f, 0.06f, 0.20f, 1.00f);

        // Header
        colors[ImGuiCol_Header]               = ImVec4(0.05f, 0.65f, 0.56f, 0.30f); // Petrol yeşili
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.96f, 0.62f, 0.04f, 0.50f); // Altın sarısı hover
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.96f, 0.25f, 0.37f, 0.70f); // Mercan pembe aktif

        // Butonlar: Mor Kadife → Altın Sarısı Hover → Mercan Pembe Aktif
        colors[ImGuiCol_Button]               = ImVec4(0.20f, 0.09f, 0.35f, 1.00f); // Mor kadife
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.96f, 0.62f, 0.04f, 0.90f); // Parlak altın sarısı
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.96f, 0.25f, 0.37f, 1.00f); // Mercan pembe

        // Sekmeler: Mor → Altın Hover → Petrol Yeşili Aktif
        colors[ImGuiCol_Tab]                  = ImVec4(0.15f, 0.07f, 0.26f, 1.00f);
        colors[ImGuiCol_TabHovered]           = ImVec4(0.96f, 0.62f, 0.04f, 0.75f); // Altın sarısı
        colors[ImGuiCol_TabActive]            = ImVec4(0.05f, 0.65f, 0.56f, 1.00f); // Petrol yeşili neon
        colors[ImGuiCol_TabUnfocused]         = ImVec4(0.12f, 0.06f, 0.20f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.05f, 0.55f, 0.48f, 0.80f);

        // Kontroller
        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.05f, 0.18f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.38f, 0.18f, 0.62f, 1.00f); // Leylak mor
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.05f, 0.65f, 0.56f, 0.80f); // Petrol yeşili
        colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.96f, 0.62f, 0.04f, 1.00f); // Altın sarısı
        colors[ImGuiCol_CheckMark]            = ImVec4(0.05f, 0.85f, 0.72f, 1.00f); // Parlak teal
        colors[ImGuiCol_SliderGrab]           = ImVec4(0.05f, 0.65f, 0.56f, 1.00f); // Petrol yeşili
        colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.96f, 0.62f, 0.04f, 1.00f); // Altın sarısı
        colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.05f, 0.02f, 0.10f, 0.70f); // Mor karartma
    }
}

// ── 7. Gelismis Konsol Cikti Tasarimcisi (Custom Print Scene) ─────────────
void RenderCustomPrintScene(ProjectManager& pm) {
    bool isEn = (pm.currentLanguage == AppLanguage::English);

    ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "CUSTOM CONSOLE OUTPUT DESIGNER" : "GELİŞMİŞ KONSOL ÇIKTI TASARIMCISI");
    ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), isEn ? "Design framed messages, banners, and formatted console layouts." : "Konsol için çerçeveli menüler, ASCII başlıklar ve süslü çıktılar tasarlayın.");
    ImGui::Separator();
    ImGui::Spacing();

    static char customMsg[256] = "SISTEM BASARIYLA CALISIYOR";
    static int borderStyle = 0; // 0: Single, 1: Double, 2: Star, 3: Hash

    ImGui::Text(isEn ? "Message Content:" : "Mesaj İçeriği:");
    ImGui::InputText("##custMsg", customMsg, sizeof(customMsg));

    ImGui::Spacing();
    ImGui::Text(isEn ? "Frame Style:" : "Çerçeve Stili:");
    const char* styles[] = { "Tek Cizgi (Single: +---+)", "Cift Cizgi (Double: #===#)", "Yildizli (Stars: *---*)", "Kare Kutusu (Bracket: [---])" };
    ImGui::Combo("##bStyle", &borderStyle, styles, IM_ARRAYSIZE(styles));

    // Canlı Konsol Önizlemesi Oluştur
    std::string line1, line2, line3;
    int len = (int)strlen(customMsg) + 4;
    if (borderStyle == 0) {
        line1 = "+" + std::string(len, '-') + "+";
        line2 = "|  " + std::string(customMsg) + "  |";
        line3 = "+" + std::string(len, '-') + "+";
    } else if (borderStyle == 1) {
        line1 = "#" + std::string(len, '=') + "#";
        line2 = "#  " + std::string(customMsg) + "  #";
        line3 = "#" + std::string(len, '=') + "#";
    } else if (borderStyle == 2) {
        line1 = "*" + std::string(len, '-') + "*";
        line2 = "*  " + std::string(customMsg) + "  *";
        line3 = "*" + std::string(len, '-') + "*";
    } else {
        line1 = "[" + std::string(len, '=') + "]";
        line2 = "[  " + std::string(customMsg) + "  ]";
        line3 = "[" + std::string(len, '=') + "]";
    }

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), isEn ? "Console Live Preview:" : "Canlı Konsol Önizlemesi:");
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.01f, 0.02f, 0.03f, 1.0f));
    ImGui::BeginChild("PrintPreviewBox", ImVec2(0, 110), true);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "%s\n%s\n%s", line1.c_str(), line2.c_str(), line3.c_str());
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Spacing();
    if (ImGui::Button(isEn ? "+ Spawn Framed Print Node to Graph" : "+ Bu Çerçeveli Yazıyı Grafiğe Düğüm Olarak Ekle", ImVec2(340, 40))) {
        FlowNode* n1 = pm.AddNode(NodeKind::Print, "Frame Top", 400, 200);
        n1->ioText = line1;
        FlowNode* n2 = pm.AddNode(NodeKind::Print, "Frame Content", 400, 310);
        n2->ioText = line2;
        FlowNode* n3 = pm.AddNode(NodeKind::Print, "Frame Bottom", 400, 420);
        n3->ioText = line3;
        pm.ConnectPins(n1->id, 1, n2->id, 0);
        pm.ConnectPins(n2->id, 1, n3->id, 0);
        pm.statusMessage = isEn ? "Framed print node chain spawned!" : "Çerçeveli çıktı düğüm zinciri eklendi!";
    }
}

// ── 8. Ayarlar Sekmesi (Settings Workspace) ──────────────────────────────
void RenderSettingsWorkspace(ProjectManager& pm) {
    bool isEn = (pm.currentLanguage == AppLanguage::English);

    ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "IDE SETTINGS & APPEARANCE" : "IDE AYARLARI VE GÖRÜNÜM");
    ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), isEn ? "Configure language, color themes, and canvas layout." : "Dil seçenekleri, tema renkleri ve çizim alanı özelleştirmeleri.");
    ImGui::Separator();
    ImGui::Spacing();

    // 1. Dil Ayarı (Language)
    ImGui::Text(isEn ? "1. Interface Language:" : "1. Arayüz Dili:");
    const char* langNames[] = { "Türkçe (Turkish)", "English" };
    int curLang = (int)pm.currentLanguage;
    if (ImGui::Combo("##lang", &curLang, langNames, IM_ARRAYSIZE(langNames))) {
        pm.currentLanguage = (AppLanguage)curLang;
        pm.statusMessage = (curLang == 0) ? "Dil Türkçe olarak ayarlandı." : "Language set to English.";
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 2. Tema Ayarı (Theme)
    ImGui::Text(isEn ? "2. Color Theme:" : "2. Arayüz Teması:");
    const char* themeNames[] = { 
        isEn ? "Light Pastel Neon (Ivory, Teal & Pink)" : "Açık Pastel Neon (Petrol Yeşili & Pembe)", 
        isEn ? "Dark Amethyst Night (Neon Purple, Teal & Gold) — Default" : "Koyu Ametist Gece (Neon Mor, Petrol Yeşili & Altın) — Varsayılan" 
    };
    int curTheme = (int)pm.currentTheme;
    if (ImGui::Combo("##theme", &curTheme, themeNames, IM_ARRAYSIZE(themeNames))) {
        pm.currentTheme = (AppTheme)curTheme;
        ApplyAppTheme(pm.currentTheme);
        pm.statusMessage = isEn ? "Theme updated." : "Tema güncellendi.";
    }

    ImGui::Spacing();
    if (ImGui::Button(isEn ? "Light Pastel Neon Theme" : "Açık Pastel Neon Temaya Geç", ImVec2(260, 32))) {
        pm.currentTheme = AppTheme::LightPastelNeon;
        ApplyAppTheme(pm.currentTheme);
        pm.statusMessage = isEn ? "Switched to Light Pastel Neon theme." : "Açık Pastel Neon temaya geçildi.";
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "Dark Amethyst Night (Default)" : "Koyu Ametist Gece (Varsayılan)", ImVec2(280, 32))) {
        pm.currentTheme = AppTheme::DarkCyberNeon;
        ApplyAppTheme(pm.currentTheme);
        pm.statusMessage = isEn ? "Switched to Dark Amethyst Night theme." : "Koyu Ametist Gece temasına geçildi.";
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 3. Akış Şeması Ayarları
    ImGui::Text(isEn ? "3. Flow Graph Grid:" : "3. Akış Şeması Izgarası:");
    ImGui::Checkbox(isEn ? "Show Background Grid" : "Arka Plan Izgarasını Göster", &pm.showGrid);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 4. Lisans ve Fikri Mülkiyet (Açık Kaynak Apache 2.0)
    ImGui::TextColored(ImVec4(0.05f, 0.85f, 0.55f, 1.0f), isEn ? "🛡️ Open Source License & Architecture:" : "🛡️ Açık Kaynak Lisansı ve Mimari:");
    ImGui::BulletText(isEn ? "Lead Developer: Mustafa Can Yılmazer (@ElictryzDev)" : "Baş Geliştirici: Mustafa Can Yılmazer (@ElictryzDev)");
    ImGui::BulletText(isEn ? "Institution: Istanbul Beykent University - Computer Programming" : "Kurum: İstanbul Beykent Üniversitesi - Bilgisayar Programcılığı");
    ImGui::BulletText(isEn ? "License: Apache License 2.0 (Permissive with Patent Grant & Retaliation Clause)" : "Lisans: Apache License 2.0 (Geniş İzinli, Patent Korumalı ve Atıf Zorunlu)");
    ImGui::BulletText(isEn ? "Freedom: You can freely inspect, modify, fork, and distribute this software." : "Özgürlük: Yazılımı inceleyebilir, değiştirebilir, çatallayabilir ve ticari projelerde dahi kullanabilirsiniz.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 5. Destek & Sponsorluk (Buy Me a Coffee / GitHub Sponsors)
    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), isEn ? "☕ Support the Developer / Community Sponsorship:" : "☕ Geliştiriciye Destek Ol / Topluluk Sponsorluğu:");
    ImGui::TextWrapped(isEn 
        ? "C-Algorithm is an independent, community-driven open source initiative for computer science education. Support further development via Buy Me a Coffee or GitHub Sponsors!" 
        : "C-Algorithm, bilgisayar programcılığı ve algoritma eğitimi için geliştirilen bağımsız bir açık kaynak projesidir. Geliştirmeyi desteklemek için bağış yapabilir veya sponsor olabilirsiniz!");

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.45f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.55f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.35f, 0.10f, 1.0f));
    if (ImGui::Button(isEn ? "☕ Buy Me a Coffee / Sponsor Developer" : "☕ Geliştiriciye Bir Kahve Ismarla (Buy Me a Coffee)", ImVec2(340, 38))) {
        ShellExecuteA(nullptr, "open", "https://buymeacoffee.com/elictryzn", nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.55f, 0.85f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.65f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.45f, 0.75f, 1.0f));
    if (ImGui::Button(isEn ? "GitHub Repository & Issues" : "GitHub Deposu ve Hata Bildirimi", ImVec2(240, 38))) {
        ShellExecuteA(nullptr, "open", "https://github.com/ElictryzDev/C-Algorithm", nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::PopStyleColor(3);
}
