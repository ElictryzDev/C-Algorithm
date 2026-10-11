#include "Workspaces.hpp"
#include <cmath>

// ── 1. Proje Karsilama Ekrani (Project Gating) ───────────────────────────
void RenderProjectGatingModal(ProjectManager& pm) {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(620, 480));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    ImGui::Begin("C-ALGORITHM — PROJE GIRIS PANELI", nullptr, flags);

    ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), "⚡ C-ALGORITHM NATIVE IDE'YE HOS GELDINIZ");
    ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), "Devam etmek icin lutfen bir proje secin veya yeni bir proje olusturun.");
    ImGui::Separator();
    ImGui::Spacing();

    static char newProjName[64] = "Algoritma_Test_01";
    static char openFilePath[256] = "";

    // Sekmeler: Yeni Proje / Var Olani Ac
    if (ImGui::BeginTabBar("GatingTabs")) {
        if (ImGui::BeginTabItem("➕ Yeni Proje Olustur")) {
            ImGui::Spacing();
            ImGui::Text("Proje Adi:");
            ImGui::InputText("##pname", newProjName, sizeof(newProjName));

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.5f, 0.7f, 0.9f, 1.0f), "Kayit Konumu: Desktop/C-Algorithm/projects/%s.calg", newProjName);
            ImGui::Spacing();

            if (ImGui::Button("🚀 Projeyi Olustur ve Basla", ImVec2(240, 42))) {
                pm.CreateNewProject(newProjName, "C:/Users/ASUS TUF GAMING/Desktop/C-Algorithm/projects");
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("📂 Kayitli Projeleri Listele & Ac")) {
            ImGui::Spacing();
            std::vector<std::string> savedFiles = pm.GetSavedProjectFiles("C:/Users/ASUS TUF GAMING/Desktop/C-Algorithm/projects");

            ImGui::Text("Bulunan Projeler (%d adet):", (int)savedFiles.size());

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
            ImGui::Text("Secilen Dosya:");
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

// ── 2. Akis Semasi Grafigi (Flow Graph Workspace) ────────────────────────
void RenderFlowGraphWorkspace(ProjectManager& pm, int& selectedNodeId) {
    ImGui::BeginChild("FlowToolbar", ImVec2(0, 44), true);
    if (ImGui::Button("+ Degisken")) {
        pm.AddNode(NodeKind::VariableDeclare, "Degisken Tanimla", 150, 150);
    }
    ImGui::SameLine();
    if (ImGui::Button("+ Ata")) {
        pm.AddNode(NodeKind::Assign, "Deger Ata", 250, 200);
    }
    ImGui::SameLine();
    if (ImGui::Button("+ Ekrana Yaz (cout)")) {
        pm.AddNode(NodeKind::Print, "Ekrana Yazdir", 350, 250);
    }
    ImGui::SameLine();
    if (ImGui::Button("🔀 Kosul (If/Else)")) {
        pm.AddNode(NodeKind::BranchIf, "Kosul (If/Else)", 400, 200);
    }
    ImGui::SameLine();
    if (ImGui::Button("🔁 While")) {
        pm.AddNode(NodeKind::LoopWhile, "While Dongusu", 450, 250);
    }
    ImGui::SameLine();
    if (ImGui::Button("🔁 Do-While")) {
        pm.AddNode(NodeKind::LoopDoWhile, "Do-While Dongusu", 500, 280);
    }
    ImGui::SameLine();
    if (ImGui::Button("🔢 For")) {
        pm.AddNode(NodeKind::LoopFor, "For Dongusu", 550, 300);
    }
    ImGui::SameLine(0, 20);
    if (ImGui::Button("💾 Projeyi Kaydet")) {
        pm.SaveProject();
    }
    ImGui::EndChild();

    // Cizim Alani
    ImGui::BeginChild("FlowCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    ImVec2 size = ImGui::GetContentRegionAvail();

    // Izgara Cizimi
    ImU32 gridCol = IM_COL32(35, 40, 50, 255);
    for (float x = 0; x < size.x; x += 32.0f) {
        drawList->AddLine(ImVec2(origin.x + x, origin.y), ImVec2(origin.x + x, origin.y + size.y), gridCol);
    }
    for (float y = 0; y < size.y; y += 32.0f) {
        drawList->AddLine(ImVec2(origin.x, origin.y + y), ImVec2(origin.x + size.x, origin.y + y), gridCol);
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
            ImVec2 p1(origin.x + fn->x + fn->pins[w.fromPin].relX, origin.y + fn->y + fn->pins[w.fromPin].relY);
            ImVec2 p2(origin.x + tn->x + tn->pins[w.toPin].relX, origin.y + tn->y + tn->pins[w.toPin].relY);

            float dx = fabsf(p2.x - p1.x) * 0.5f;
            if (dx < 50.0f) dx = 50.0f;
            ImVec2 c1(p1.x + dx, p1.y);
            ImVec2 c2(p2.x - dx, p2.y);

            // Alt tusuna basiliyken kablonun orta noktasina yaklasilinca kirmizi makas uyarisi ver
            ImVec2 midPt((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);
            float distToMid = hypotf(mousePos.x - midPt.x, mousePos.y - midPt.y);

            bool wireHovered = (distToMid < 25.0f);

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
        ImVec2 pMin(origin.x + n.x, origin.y + n.y);
        ImVec2 pMax(pMin.x + n.width, pMin.y + n.height);

        bool isNodeHovered = (mousePos.x >= pMin.x && mousePos.x <= pMax.x && mousePos.y >= pMin.y && mousePos.y <= pMax.y);
        bool isSelected = (selectedNodeId == n.id || multiSelectedNodes.count(n.id) > 0);

        // Dugum Karti Govdesi
        drawList->AddRectFilled(pMin, pMax, IM_COL32(20, 24, 30, 255), 8.0f);
        ImU32 borderCol = isSelected ? IM_COL32(0, 210, 255, 255) : IM_COL32(45, 52, 64, 255);
        drawList->AddRect(pMin, pMax, borderCol, 8.0f, 0, isSelected ? 2.5f : 1.2f);

        // Baslik Seridi
        ImU32 headCol = (n.kind == NodeKind::Start)           ? IM_COL32(28, 75, 45, 255) :
                        (n.kind == NodeKind::End)             ? IM_COL32(85, 30, 30, 255) :
                        (n.kind == NodeKind::VariableDeclare) ? IM_COL32(25, 58, 80, 255) :
                        (n.kind == NodeKind::BranchIf)        ? IM_COL32(120, 65, 25, 255) :
                        (n.kind == NodeKind::LoopWhile || n.kind == NodeKind::LoopDoWhile || n.kind == NodeKind::LoopFor) ? IM_COL32(75, 35, 95, 255) :
                        IM_COL32(35, 42, 52, 255);

        drawList->AddRectFilled(pMin, ImVec2(pMax.x, pMin.y + 26.0f), headCol, 8.0f, ImDrawFlags_RoundCornersTop);
        drawList->AddText(ImVec2(pMin.x + 12, pMin.y + 5), IM_COL32(245, 250, 255, 255), n.title.c_str());

        // Pinleri Ciz ve Pin Uzerinde Olup Olmadigini (Hit-Test) Kontrol Et
        for (size_t pIdx = 0; pIdx < n.pins.size(); pIdx++) {
            const auto& pin = n.pins[pIdx];
            ImVec2 pinPos(pMin.x + pin.relX, pMin.y + pin.relY);

            float distToPin = hypotf(mousePos.x - pinPos.x, mousePos.y - pinPos.y);
            bool isPinHovered = (distToPin <= 10.0f);

            if (isPinHovered) {
                hoveredPinNode = n.id;
                hoveredPinIdx = (int)pIdx;
            }

            ImU32 pCol = (pin.type == PinType::ExecIn || pin.type == PinType::ExecOut) ? IM_COL32(0, 200, 232, 255) : IM_COL32(63, 185, 80, 255);
            if (isPinHovered) pCol = IM_COL32(255, 255, 100, 255);

            if (pin.shape == PinShape::Diamond) {
                float s = 6.0f;
                drawList->AddQuadFilled(
                    ImVec2(pinPos.x, pinPos.y - s),
                    ImVec2(pinPos.x + s, pinPos.y),
                    ImVec2(pinPos.x, pinPos.y + s),
                    ImVec2(pinPos.x - s, pinPos.y),
                    isPinHovered ? IM_COL32(255, 255, 100, 255) : IM_COL32(255, 165, 0, 255)
                );
            } else if (pin.shape == PinShape::Triangle) {
                float s = 6.0f;
                drawList->AddTriangleFilled(
                    ImVec2(pinPos.x - s, pinPos.y - s),
                    ImVec2(pinPos.x + s, pinPos.y),
                    ImVec2(pinPos.x - s, pinPos.y + s),
                    pCol
                );
            } else {
                drawList->AddCircleFilled(pinPos, 6.0f, pCol);
            }

            drawList->AddCircle(pinPos, 7.0f, IM_COL32(255, 255, 255, isPinHovered ? 255 : 120));

            // Pin Ismi (Temiz hizalanmis)
            if (pin.type == PinType::ExecIn) {
                drawList->AddText(ImVec2(pinPos.x + 12, pinPos.y - 8), IM_COL32(180, 190, 205, 255), pin.name.c_str());
            } else {
                ImVec2 textSize = ImGui::CalcTextSize(pin.name.c_str());
                drawList->AddText(ImVec2(pinPos.x - textSize.x - 12, pinPos.y - 8), IM_COL32(180, 190, 205, 255), pin.name.c_str());
            }
        }

        // Dugum Surukleme & Secim Tetikleyicisi (Yalnizca pine basilmiyorsa!)
        if (ImGui::IsMouseClicked(0) && isNodeHovered && hoveredPinNode == -1) {
            selectedNodeId = n.id;
            multiSelectedNodes.clear();
            multiSelectedNodes.insert(n.id);
            draggingId = n.id;
            dragOffset = ImVec2(mousePos.x - n.x, mousePos.y - n.y);
        }

        if (draggingId == n.id) {
            if (ImGui::IsMouseDown(0)) {
                n.x = mousePos.x - dragOffset.x;
                n.y = mousePos.y - dragOffset.y;
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
            wireDragStartPos = ImVec2(origin.x + fn->x + fn->pins[hoveredPinIdx].relX, 
                                      origin.y + fn->y + fn->pins[hoveredPinIdx].relY);
        }
    }

    if (wireDragFromNode > 0) {
        if (ImGui::IsMouseDown(0)) {
            // Farenin ucuna dogru dinamik kablo ciz
            float dx = fabsf(mousePos.x - wireDragStartPos.x) * 0.5f;
            if (dx < 50.0f) dx = 50.0f;
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
    bool isCanvasHovered = ImGui::IsWindowHovered();
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
                float nLeft = origin.x + n.x;
                float nRight = nLeft + n.width;
                float nTop = origin.y + n.y;
                float nBottom = nTop + n.height;

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
        contextMenuPos = ImVec2(mousePos.x - origin.x, mousePos.y - origin.y);
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

    ImGui::BeginChild("CodegenBar", ImVec2(0, 42), true);
    if (ImGui::Button("📋 Panoya Kopyala (Copy Code)")) {
        ImGui::SetClipboardText(code.c_str());
        pm.statusMessage = "C++ Kodu panoya kopyalandi!";
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "C++17 Standartlarinda Derlenebilir Kod");
    ImGui::EndChild();

    ImGui::BeginChild("CodegenView", ImVec2(0, 0), true);
    ImGui::InputTextMultiline("##cppsrc", (char*)code.c_str(), code.size(), ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
    ImGui::EndChild();
}

// ── 5. Inspector Panel ───────────────────────────────────────────────────
void RenderInspectorPanel(ProjectManager& pm, int& selectedNodeId) {
    ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), "🔍 DUGUM OZELLIKLERI");
    ImGui::Separator();

    if (selectedNodeId > 0) {
        FlowNode* n = pm.GetNode(selectedNodeId);
        if (n) {
            ImGui::Text("Dugum: #%d (%s)", n->id, n->title.c_str());

            if (n->kind == NodeKind::VariableDeclare) {
                char vName[64];
                strncpy(vName, n->targetVar.c_str(), sizeof(vName));
                if (ImGui::InputText("Degisken Adi", vName, sizeof(vName))) {
                    n->targetVar = vName;
                }

                // Tip Secici (Zengin Tipler)
                const char* typeNames[] = { "int (Int32)", "long long (Int64)", "float", "double", "bool", "uint8_t (0-255)", "uint16_t", "uint32_t", "uint64_t", "std::string" };
                int currentType = (int)n->varType;
                if (ImGui::Combo("Veri Turu", &currentType, typeNames, IM_ARRAYSIZE(typeNames))) {
                    n->varType = (TypeKind)currentType;
                }

                char valBuf[64];
                strncpy(valBuf, n->expr.c_str(), sizeof(valBuf));
                if (ImGui::InputText("Baslangic Degeri", valBuf, sizeof(valBuf))) {
                    n->expr = valBuf;
                }
            } else if (n->kind == NodeKind::Print) {
                char textBuf[128];
                strncpy(textBuf, n->ioText.c_str(), sizeof(textBuf));
                if (ImGui::InputText("Yazdirilacak Metin", textBuf, sizeof(textBuf))) {
                    n->ioText = textBuf;
                }
            } else if (n->kind == NodeKind::Assign) {
                char varBuf[64];
                strncpy(varBuf, n->targetVar.c_str(), sizeof(varBuf));
                if (ImGui::InputText("Hedef Degisken", varBuf, sizeof(varBuf))) {
                    n->targetVar = varBuf;
                }
                char exprBuf[64];
                strncpy(exprBuf, n->expr.c_str(), sizeof(exprBuf));
                if (ImGui::InputText("Atanacak Deger / Ifade", exprBuf, sizeof(exprBuf))) {
                    n->expr = exprBuf;
                }
            } else if (n->kind == NodeKind::BranchIf || n->kind == NodeKind::LoopWhile || n->kind == NodeKind::LoopDoWhile) {
                char condBuf[128];
                strncpy(condBuf, n->conditionExpr.c_str(), sizeof(condBuf));
                if (ImGui::InputText("Mantiksal Kosul (Condition)", condBuf, sizeof(condBuf))) {
                    n->conditionExpr = condBuf;
                }
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Ornek: x < 10, can <= 0, puan >= 100");
            } else if (n->kind == NodeKind::LoopFor) {
                char initBuf[64];
                strncpy(initBuf, n->loopInit.c_str(), sizeof(initBuf));
                if (ImGui::InputText("Baslangic (Init)", initBuf, sizeof(initBuf))) {
                    n->loopInit = initBuf;
                }
                char condBuf[128];
                strncpy(condBuf, n->conditionExpr.c_str(), sizeof(condBuf));
                if (ImGui::InputText("Kosul (Condition)", condBuf, sizeof(condBuf))) {
                    n->conditionExpr = condBuf;
                }
                char stepBuf[64];
                strncpy(stepBuf, n->loopStep.c_str(), sizeof(stepBuf));
                if (ImGui::InputText("Artis (Step)", stepBuf, sizeof(stepBuf))) {
                    n->loopStep = stepBuf;
                }
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Ornek: int i = 0; i < 10; i++");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // DUGUM SIL BUTONU
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            if (ImGui::Button("🗑 Dugumu Sil (Delete Node)", ImVec2(-1, 32))) {
                pm.DeleteNode(selectedNodeId);
                selectedNodeId = 0;
            }
            ImGui::PopStyleColor(3);
        }
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Duzenlemek icin akistan bir dugume tiklayin.");
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
    } else if (theme == AppTheme::ClassicDark) {
        ImGui::StyleColorsDark();
    } else if (theme == AppTheme::Light) {
        ImGui::StyleColorsLight();
    }
}

// ── 7. Ayarlar Sekmesi (Settings Workspace) ──────────────────────────────
void RenderSettingsWorkspace(ProjectManager& pm) {
    ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), "⚙️ IDE AYARLARI & GORUNUM (SETTINGS)");
    ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), "Dil secenekleri, tema renkleri ve cizim alani ozellestirmeleri.");
    ImGui::Separator();
    ImGui::Spacing();

    // 1. Dil Ayarı (Language)
    ImGui::Text("🌐 1. Dil Secimi (Interface Language):");
    const char* langNames[] = { "Turkce (Turkish)", "English" };
    int curLang = (int)pm.currentLanguage;
    if (ImGui::Combo("##lang", &curLang, langNames, IM_ARRAYSIZE(langNames))) {
        pm.currentLanguage = (AppLanguage)curLang;
        pm.statusMessage = (curLang == 0) ? "Dil Turkce olarak ayarlandi." : "Language set to English.";
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 2. Tema Ayarı (Theme)
    ImGui::Text("🎨 2. Arayuz Temasi (Color Theme):");
    const char* themeNames[] = { "OLED Siyah & Turkuaz (OLED Dark)", "Klasik Koyu (Classic Dark)", "Acik Tema (Light Theme)" };
    int curTheme = (int)pm.currentTheme;
    if (ImGui::Combo("##theme", &curTheme, themeNames, IM_ARRAYSIZE(themeNames))) {
        pm.currentTheme = (AppTheme)curTheme;
        ApplyAppTheme(pm.currentTheme);
        pm.statusMessage = "Tema guncellendi.";
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 3. Akış Şeması Ayarları
    ImGui::Text("📐 3. Akis Semasi Izgarasi (Flow Grid):");
    ImGui::Checkbox("Arka Plan Izgarasini Goster (Show Grid)", &pm.showGrid);

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
