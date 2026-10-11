#include "Workspaces.hpp"
#include <cmath>

// ── 1. Proje Karsilama Ekrani (Project Gating) ───────────────────────────
void RenderProjectGatingModal(ProjectManager& pm) {
    bool isEn = (pm.currentLanguage == AppLanguage::English);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(620, 480));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    ImGui::Begin(isEn ? "C-ALGORITHM — PROJECT GATEWAY" : "C-ALGORITHM — PROJE GİRİŞ PANELİ", nullptr, flags);

    ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "WELCOME TO C-ALGORITHM NATIVE IDE" : "C-ALGORITHM NATIVE IDE'YE HOŞ GELDİNİZ");
    ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), isEn ? "Select an existing project or create a new one to continue." : "Devam etmek için lütfen bir proje seçin veya yeni bir proje oluşturun.");
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

// ── 2. Akis Semasi Grafigi (Flow Graph Workspace) ────────────────────────
void RenderFlowGraphWorkspace(ProjectManager& pm, int& selectedNodeId) {
    bool isEn = (pm.currentLanguage == AppLanguage::English);

    ImGui::BeginChild("FlowToolbar", ImVec2(0, 44), true);
    if (ImGui::Button(isEn ? "+ Variable" : "+ Değişken")) {
        pm.AddNode(NodeKind::VariableDeclare, isEn ? "Variable Declare" : "Değişken Tanımla", 150, 150);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "+ Struct" : "+ Struct")) {
        pm.AddNode(NodeKind::StructDef, isEn ? "Struct Definition" : "Struct Tanımla", 180, 170);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "+ Data Structures v" : "+ Veri Yapıları v")) {
        ImGui::OpenPopup("ToolbarDataStructuresPopup");
    }
    if (ImGui::BeginPopup("ToolbarDataStructuresPopup")) {
        ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "Data Structures (Veri Yapilari):" : "Veri Yapıları:");
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
    if (ImGui::Button(isEn ? "+ Print (cout)" : "+ Yazdır (cout)")) {
        pm.AddNode(NodeKind::Print, isEn ? "Print Output" : "Ekrana Yazdır", 350, 250);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "Branch (If)" : "Koşul (If)")) {
        pm.AddNode(NodeKind::BranchIf, isEn ? "Branch (If/Else)" : "Koşul (If/Else)", 400, 200);
    }
    ImGui::SameLine();
    if (ImGui::Button(isEn ? "Switch" : "Switch-Case")) {
        pm.AddNode(NodeKind::SwitchCase, isEn ? "Switch Case" : "Switch-Case", 430, 220);
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

        // Farenin altindaki dunya noktasini hesapla
        float worldMouseX = (mousePos.x - screenOrigin.x - g_flowPanOffset.x) / prevZoom;
        float worldMouseY = (mousePos.y - screenOrigin.y - g_flowPanOffset.y) / prevZoom;

        // Yeni zoom seviyesinde imlecin altindaki dunya noktasini kilitle
        g_flowPanOffset.x = (mousePos.x - screenOrigin.x) - worldMouseX * g_flowZoomLevel;
        g_flowPanOffset.y = (mousePos.y - screenOrigin.y) - worldMouseY * g_flowZoomLevel;
    }

    // 3. Sonsuz Izgara Cizimi (Sifir Ek Bellek Maliyetli Modulo Geometrisi)
    if (pm.showGrid) {
        ImU32 gridCol = IM_COL32(35, 40, 50, 255);
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

    // Bezier Tellerini Ciz ve Alt + Sol Tik ile Koparma Kontrolu
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
            ImVec2 c1(p1.x + dx, p1.y);
            ImVec2 c2(p2.x - dx, p2.y);

            // Alt tusuna basiliyken kablonun orta noktasina yaklasilinca kirmizi makas uyarisi ver
            ImVec2 midPt((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);
            float distToMid = hypotf(mousePos.x - midPt.x, mousePos.y - midPt.y);

            bool wireHovered = (distToMid < 25.0f * g_flowZoomLevel);

            if (wireHovered && isAltDown) {
                drawList->AddBezierCubic(p1, c1, c2, p2, IM_COL32(255, 60, 60, 255), 4.0f);
                drawList->AddCircleFilled(midPt, 7.0f, IM_COL32(255, 30, 30, 255));
                drawList->AddText(ImVec2(midPt.x + 10, midPt.y - 10), IM_COL32(255, 100, 100, 255), "[Kabloyu Kes]");

                if (ImGui::IsMouseClicked(0)) {
                    wireToDelete = w.id;
                }
            } else {
                drawList->AddBezierCubic(p1, c1, c2, p2, IM_COL32(0, 120, 200, 70), 5.0f);
                drawList->AddBezierCubic(p1, c1, c2, p2, IM_COL32(0, 200, 232, 255), 2.5f);
            }
        }
    }

    if (wireToDelete > 0) {
        pm.DisconnectWire(wireToDelete);
        pm.statusMessage = "Kablo baglantisi koparildi (Alt + Tik).";
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

    // Sag Tik Menusu Icin Tiklanan Nokta
    static ImVec2 contextMenuPos = { 0, 0 };

    // ── 1. Dugumlerin ve Pinlerin Cizimi ─────────────────────────────────
    int hoveredPinNode = -1;
    int hoveredPinIdx = -1;

    for (auto& n : pm.nodes) {
        ImVec2 pMin(screenOrigin.x + g_flowPanOffset.x + n.x * g_flowZoomLevel,
                    screenOrigin.y + g_flowPanOffset.y + n.y * g_flowZoomLevel);
        ImVec2 pMax(pMin.x + n.width * g_flowZoomLevel,
                    pMin.y + n.height * g_flowZoomLevel);

        bool isNodeHovered = (mousePos.x >= pMin.x && mousePos.x <= pMax.x && mousePos.y >= pMin.y && mousePos.y <= pMax.y);
        bool isSelected = (selectedNodeId == n.id || multiSelectedNodes.count(n.id) > 0);

        // Dugum Karti Govdesi
        drawList->AddRectFilled(pMin, pMax, IM_COL32(20, 24, 30, 255), 8.0f * g_flowZoomLevel);
        ImU32 borderCol = isSelected ? IM_COL32(0, 210, 255, 255) : IM_COL32(45, 52, 64, 255);
        drawList->AddRect(pMin, pMax, borderCol, 8.0f * g_flowZoomLevel, 0, isSelected ? 2.5f : 1.2f);

        // Baslik Seridi
        ImU32 headCol = (n.kind == NodeKind::Start)           ? IM_COL32(28, 75, 45, 255) :
                        (n.kind == NodeKind::End)             ? IM_COL32(85, 30, 30, 255) :
                        (n.kind == NodeKind::VariableDeclare) ? IM_COL32(25, 58, 80, 255) :
                        (n.kind == NodeKind::StructDef)       ? IM_COL32(80, 50, 20, 255) :
                        (n.kind == NodeKind::VectorOp)        ? IM_COL32(20, 70, 85, 255) :
                        (n.kind == NodeKind::ListOp)          ? IM_COL32(25, 80, 65, 255) :
                        (n.kind == NodeKind::StackOp)         ? IM_COL32(70, 35, 85, 255) :
                        (n.kind == NodeKind::QueueOp)         ? IM_COL32(95, 60, 20, 255) :
                        (n.kind == NodeKind::ArrayOp)         ? IM_COL32(35, 65, 95, 255) :
                        (n.kind == NodeKind::BranchIf)        ? IM_COL32(120, 65, 25, 255) :
                        (n.kind == NodeKind::LoopWhile || n.kind == NodeKind::LoopDoWhile || n.kind == NodeKind::LoopFor) ? IM_COL32(75, 35, 95, 255) :
                        IM_COL32(35, 42, 52, 255);

        drawList->AddRectFilled(pMin, ImVec2(pMax.x, pMin.y + 26.0f * g_flowZoomLevel), headCol, 8.0f * g_flowZoomLevel, ImDrawFlags_RoundCornersTop);
        drawList->AddText(ImVec2(pMin.x + 12 * g_flowZoomLevel, pMin.y + 5), IM_COL32(245, 250, 255, 255), n.title.c_str());

        // Pinleri Ciz ve Pin Uzerinde Olup Olmadigini (Hit-Test) Kontrol Et
        for (size_t pIdx = 0; pIdx < n.pins.size(); pIdx++) {
            const auto& pin = n.pins[pIdx];
            ImVec2 pinPos(pMin.x + pin.relX * g_flowZoomLevel, pMin.y + pin.relY * g_flowZoomLevel);

            float distToPin = hypotf(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
            bool isPinHovered = (distToPin <= 10.0f * g_flowZoomLevel);

            if (isPinHovered) {
                hoveredPinNode = n.id;
                hoveredPinIdx = (int)pIdx;
            }

            ImU32 pCol = (pin.type == PinType::ExecIn || pin.type == PinType::ExecOut) ? IM_COL32(0, 200, 232, 255) : IM_COL32(63, 185, 80, 255);
            if (isPinHovered) pCol = IM_COL32(255, 255, 100, 255);

            if (pin.shape == PinShape::Diamond) {
                float s = 6.0f * g_flowZoomLevel;
                drawList->AddQuadFilled(
                    ImVec2(pinPos.x, pinPos.y - s),
                    ImVec2(pinPos.x + s, pinPos.y),
                    ImVec2(pinPos.x, pinPos.y + s),
                    ImVec2(pinPos.x - s, pinPos.y),
                    isPinHovered ? IM_COL32(255, 255, 100, 255) : IM_COL32(255, 165, 0, 255)
                );
            } else if (pin.shape == PinShape::Triangle) {
                float s = 6.0f * g_flowZoomLevel;
                drawList->AddTriangleFilled(
                    ImVec2(pinPos.x - s, pinPos.y - s),
                    ImVec2(pinPos.x + s, pinPos.y),
                    ImVec2(pinPos.x - s, pinPos.y + s),
                    pCol
                );
            } else {
                drawList->AddCircleFilled(pinPos, 6.0f * g_flowZoomLevel, pCol);
            }

            drawList->AddCircle(pinPos, 7.0f * g_flowZoomLevel, IM_COL32(255, 255, 255, isPinHovered ? 255 : 120));

            // Pin Ismi (Temiz hizalanmis)
            if (pin.type == PinType::ExecIn) {
                drawList->AddText(ImVec2(pinPos.x + 12 * g_flowZoomLevel, pinPos.y - 8), IM_COL32(180, 190, 205, 255), pin.name.c_str());
            } else {
                ImVec2 textSize = ImGui::CalcTextSize(pin.name.c_str());
                drawList->AddText(ImVec2(pinPos.x - textSize.x - 12 * g_flowZoomLevel, pinPos.y - 8), IM_COL32(180, 190, 205, 255), pin.name.c_str());
            }
        }

        // Dugum Surukleme & Secim Tetikleyicisi (Yalnizca pine basilmiyorsa!)
        if (ImGui::IsMouseClicked(0) && isNodeHovered && hoveredPinNode == -1) {
            selectedNodeId = n.id;
            multiSelectedNodes.clear();
            multiSelectedNodes.insert(n.id);
            draggingId = n.id;
            dragOffset = ImVec2(mousePos.x - pMin.x, mousePos.y - pMin.y);
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

    // ── 2. Pinden Canli Kablo Cekme Etkilesimi ─────────────────────────────
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
            // Farenin ucuna dogru dinamik kablo ciz
            float dx = fabsf(mousePos.x - wireDragStartPos.x) * 0.5f;
            if (dx < 40.0f * g_flowZoomLevel) dx = 40.0f * g_flowZoomLevel;
            ImVec2 c1(wireDragStartPos.x + dx, wireDragStartPos.y);
            ImVec2 c2(mousePos.x - dx, mousePos.y);

            drawList->AddBezierCubic(wireDragStartPos, c1, c2, mousePos, IM_COL32(255, 220, 50, 100), 5.0f);
            drawList->AddBezierCubic(wireDragStartPos, c1, c2, mousePos, IM_COL32(255, 255, 100, 255), 2.5f);
            drawList->AddCircleFilled(mousePos, 5.0f, IM_COL32(255, 255, 100, 255));
        } else {
            // Fare birakildi! Hedef pin var mi kontrol et
            if (hoveredPinNode > 0 && hoveredPinNode != wireDragFromNode) {
                pm.ConnectPins(wireDragFromNode, wireDragFromPin, hoveredPinNode, hoveredPinIdx);
            }
            wireDragFromNode = -1;
            wireDragFromPin = -1;
        }
    }

    // ── 3. Bos Alan Tiklamasi: Kutu Secimi (Marquee Selection) ────────────
    if (ImGui::IsMouseClicked(0) && isCanvasHovered && draggingId == -1 && wireDragFromNode == -1 && hoveredPinNode == -1) {
        // Herhangi bir dugume tiklanmadiysa kutu secimini baslat
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

            // Kutunun icindeki dugumleri sec
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
        ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), "Dugum Ekle (Spawn Node)");
        ImGui::Separator();

        if (ImGui::MenuItem("Event Software Start")) {
            pm.AddNode(NodeKind::Start, "Event Software Start", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem("Variable Declare (Tanimla)")) {
            pm.AddNode(NodeKind::VariableDeclare, "Variable Declare", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem("Struct Definition (Veri Yapisi)")) {
            pm.AddNode(NodeKind::StructDef, "Struct Definition", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::BeginMenu(isEn ? "Data Structures (Veri Yapilari)" : "Veri Yapıları (Data Structures)")) {
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
            if (ImGui::MenuItem("Sabit Dizi (std::array / Dizi)")) {
                pm.AddNode(NodeKind::ArrayOp, "Array (Dizi)", contextMenuPos.x, contextMenuPos.y);
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Assign Value (Deger Ata)")) {
            pm.AddNode(NodeKind::Assign, "Assign Value", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem("Print Output (cout)")) {
            pm.AddNode(NodeKind::Print, "Print Output", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem("Input Read (cin)")) {
            pm.AddNode(NodeKind::Input, "Input Read", contextMenuPos.x, contextMenuPos.y);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Branch Condition (If/Else)")) {
            pm.AddNode(NodeKind::BranchIf, "Branch (If/Else)", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem("Switch Case")) {
            pm.AddNode(NodeKind::SwitchCase, "Switch Case", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem("While Loop")) {
            pm.AddNode(NodeKind::LoopWhile, "While Loop", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem("Do-While Loop")) {
            pm.AddNode(NodeKind::LoopDoWhile, "Do-While Loop", contextMenuPos.x, contextMenuPos.y);
        }
        if (ImGui::MenuItem("For Loop")) {
            pm.AddNode(NodeKind::LoopFor, "For Loop", contextMenuPos.x, contextMenuPos.y);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Program Return (0)")) {
            pm.AddNode(NodeKind::End, "Program Return (0)", contextMenuPos.x, contextMenuPos.y);
        }

        ImGui::EndPopup();
    }

    ImGui::EndChild();
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

// ── 4. Canli C++ Kodu Workspace ──────────────────────────────────────────
void RenderCodegenWorkspace(ProjectManager& pm) {
    std::string code = pm.GenerateCppCode();

    bool isEn = (pm.currentLanguage == AppLanguage::English);

    ImGui::BeginChild("CodegenBar", ImVec2(0, 42), true);
    if (ImGui::Button(isEn ? "Copy to Clipboard" : "Panoya Kopyala")) {
        ImGui::SetClipboardText(code.c_str());
        pm.statusMessage = isEn ? "C++ Code copied to clipboard!" : "C++ Kodu panoya kopyalandi!";
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), isEn ? "Compilable C++17 Standard Code" : "C++17 Standartlarinda Derlenebilir Kod");
    ImGui::EndChild();

    ImGui::BeginChild("CodegenView", ImVec2(0, 0), true);
    // Null-terminator guvenligi: code.size() + 1
    ImGui::InputTextMultiline("##cppsrc", (char*)code.c_str(), code.size() + 1, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
    ImGui::EndChild();
}

// ── 5. Inspector Panel ───────────────────────────────────────────────────
void RenderInspectorPanel(ProjectManager& pm, int& selectedNodeId) {
    bool isEn = (pm.currentLanguage == AppLanguage::English);

    ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), isEn ? "NODE INSPECTOR" : "DÜĞÜM ÖZELLİKLERİ");
    ImGui::Separator();

    if (selectedNodeId > 0) {
        FlowNode* n = pm.GetNode(selectedNodeId);
        if (n) {
            ImGui::Text(isEn ? "Node: #%d (%s)" : "Düğüm: #%d (%s)", n->id, n->title.c_str());

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
                char textBuf[128];
                strncpy(textBuf, n->ioText.c_str(), sizeof(textBuf));
                if (ImGui::InputText(isEn ? "Text to Print" : "Yazdırılacak Metin", textBuf, sizeof(textBuf))) {
                    n->ioText = textBuf;
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
            } else if (n->kind == NodeKind::BranchIf || n->kind == NodeKind::LoopWhile || n->kind == NodeKind::LoopDoWhile) {
                char condBuf[128];
                strncpy(condBuf, n->conditionExpr.c_str(), sizeof(condBuf));
                if (ImGui::InputText(isEn ? "Condition" : "Mantıksal Koşul", condBuf, sizeof(condBuf))) {
                    n->conditionExpr = condBuf;
                }
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), isEn ? "Example: x < 10, hp <= 0" : "Örnek: x < 10, can <= 0, puan >= 100");
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
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            if (ImGui::Button(isEn ? "Delete Node" : "Düğümü Sil", ImVec2(-1, 32))) {
                pm.DeleteNode(selectedNodeId);
                selectedNodeId = 0;
            }
            ImGui::PopStyleColor(3);
        }
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), isEn ? "Click on a node in the graph to inspect." : "Düzenlemek için akıştan bir düğüme tıklayın.");
    }
}

// ── 6. Tema ve Stil Yonetimi ─────────────────────────────────────────────
void ApplyAppTheme(AppTheme theme) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    if (theme == AppTheme::OledDark) {
        ImGui::StyleColorsDark();
        colors[ImGuiCol_WindowBg]           = ImVec4(0.04f, 0.05f, 0.07f, 1.00f);
        colors[ImGuiCol_Header]             = ImVec4(0.10f, 0.15f, 0.22f, 1.00f);
        colors[ImGuiCol_HeaderHovered]      = ImVec4(0.00f, 0.60f, 0.85f, 0.70f);
        colors[ImGuiCol_Button]             = ImVec4(0.12f, 0.16f, 0.24f, 1.00f);
        colors[ImGuiCol_ButtonHovered]      = ImVec4(0.00f, 0.70f, 0.95f, 0.80f);
        colors[ImGuiCol_ButtonActive]       = ImVec4(0.00f, 0.50f, 0.75f, 1.00f);
        colors[ImGuiCol_FrameBg]            = ImVec4(0.08f, 0.10f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.15f, 0.20f, 0.28f, 1.00f);
        colors[ImGuiCol_Tab]                = ImVec4(0.07f, 0.10f, 0.14f, 1.00f);
        colors[ImGuiCol_TabHovered]         = ImVec4(0.00f, 0.70f, 0.95f, 0.80f);
        colors[ImGuiCol_TabActive]          = ImVec4(0.00f, 0.55f, 0.80f, 1.00f);
    } else if (theme == AppTheme::Dracula) {
        ImGui::StyleColorsDark();
        colors[ImGuiCol_WindowBg]           = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
        colors[ImGuiCol_Header]             = ImVec4(0.38f, 0.31f, 0.55f, 1.00f);
        colors[ImGuiCol_HeaderHovered]      = ImVec4(0.74f, 0.46f, 0.88f, 0.80f);
        colors[ImGuiCol_Button]             = ImVec4(0.24f, 0.25f, 0.34f, 1.00f);
        colors[ImGuiCol_ButtonHovered]      = ImVec4(0.55f, 0.45f, 0.75f, 1.00f);
        colors[ImGuiCol_ButtonActive]       = ImVec4(0.40f, 0.30f, 0.60f, 1.00f);
        colors[ImGuiCol_FrameBg]            = ImVec4(0.20f, 0.21f, 0.28f, 1.00f);
        colors[ImGuiCol_Tab]                = ImVec4(0.18f, 0.19f, 0.25f, 1.00f);
        colors[ImGuiCol_TabHovered]         = ImVec4(0.74f, 0.46f, 0.88f, 0.80f);
        colors[ImGuiCol_TabActive]          = ImVec4(0.55f, 0.45f, 0.75f, 1.00f);
    } else if (theme == AppTheme::Cyberpunk) {
        ImGui::StyleColorsDark();
        colors[ImGuiCol_WindowBg]           = ImVec4(0.05f, 0.02f, 0.09f, 1.00f);
        colors[ImGuiCol_Header]             = ImVec4(0.85f, 0.00f, 0.55f, 0.70f);
        colors[ImGuiCol_HeaderHovered]      = ImVec4(1.00f, 0.80f, 0.00f, 0.80f);
        colors[ImGuiCol_Button]             = ImVec4(0.30f, 0.05f, 0.35f, 1.00f);
        colors[ImGuiCol_ButtonHovered]      = ImVec4(0.95f, 0.10f, 0.55f, 1.00f);
        colors[ImGuiCol_ButtonActive]       = ImVec4(1.00f, 0.85f, 0.00f, 1.00f);
        colors[ImGuiCol_FrameBg]            = ImVec4(0.15f, 0.05f, 0.20f, 1.00f);
        colors[ImGuiCol_Tab]                = ImVec4(0.18f, 0.05f, 0.22f, 1.00f);
        colors[ImGuiCol_TabHovered]         = ImVec4(0.95f, 0.10f, 0.55f, 0.80f);
        colors[ImGuiCol_TabActive]          = ImVec4(0.80f, 0.00f, 0.50f, 1.00f);
    } else if (theme == AppTheme::ClassicDark) {
        ImGui::StyleColorsDark();
    } else if (theme == AppTheme::Light) {
        ImGui::StyleColorsLight();
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
    const char* themeNames[] = { "OLED Siyah & Turkuaz", "Klasik Koyu (Classic Dark)", "Dracula (Mor & Lavanta)", "Cyberpunk (Neon Pembe)", "Açık Tema (Light Theme)" };
    int curTheme = (int)pm.currentTheme;
    if (ImGui::Combo("##theme", &curTheme, themeNames, IM_ARRAYSIZE(themeNames))) {
        pm.currentTheme = (AppTheme)curTheme;
        ApplyAppTheme(pm.currentTheme);
        pm.statusMessage = isEn ? "Theme updated." : "Tema güncellendi.";
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

    // 4. Lisans ve Fikri Mulkiyet Bilgisi
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "🛡️ Lisans ve Gelistirici Bilgisi:");
    ImGui::BulletText("Gelistirici: Mustafa Can Yilmazer (ElictryzDev)");
    ImGui::BulletText("Kurum: Istanbul Beykent Universitesi - Bilgisayar Programciligi");
    ImGui::BulletText("Lisans: Proprietary EULA (Tum Haklari Saklidir)");
    ImGui::BulletText("Cikti Kodu: Ticari projelerde 'C-Algorithm' atfi ile kullanilabilir.");
}
