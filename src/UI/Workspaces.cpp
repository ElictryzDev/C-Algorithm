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

    // Bezier Tellerini Ciz
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

            drawList->AddBezierCubic(p1, c1, c2, p2, IM_COL32(0, 120, 200, 70), 5.0f);
            drawList->AddBezierCubic(p1, c1, c2, p2, IM_COL32(0, 200, 232, 255), 2.5f);
        }
    }

    // Dugumleri Ciz ve Surukle
    static int draggingId = -1;
    static ImVec2 dragOffset = { 0, 0 };

    for (auto& n : pm.nodes) {
        ImVec2 pMin(origin.x + n.x, origin.y + n.y);
        ImVec2 pMax(pMin.x + n.width, pMin.y + n.height);

        bool isHovered = (mousePos.x >= pMin.x && mousePos.x <= pMax.x && mousePos.y >= pMin.y && mousePos.y <= pMax.y);
        bool isSelected = (selectedNodeId == n.id);

        if (ImGui::IsMouseClicked(0) && isHovered) {
            selectedNodeId = n.id;
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

        // Dugum Karti Govdesi
        drawList->AddRectFilled(pMin, pMax, IM_COL32(22, 27, 34, 255), 8.0f);
        ImU32 borderCol = isSelected ? IM_COL32(0, 200, 232, 255) : IM_COL32(48, 54, 61, 255);
        drawList->AddRect(pMin, pMax, borderCol, 8.0f, 0, isSelected ? 2.5f : 1.2f);

        // Baslik Seridi
        ImU32 headCol = (n.kind == NodeKind::Start)           ? IM_COL32(25, 65, 40, 255) :
                        (n.kind == NodeKind::End)             ? IM_COL32(75, 25, 25, 255) :
                        (n.kind == NodeKind::VariableDeclare) ? IM_COL32(20, 50, 70, 255) :
                        (n.kind == NodeKind::BranchIf)        ? IM_COL32(110, 60, 20, 255) :  // Turuncu/Kehribar Kosul
                        (n.kind == NodeKind::LoopWhile || n.kind == NodeKind::LoopDoWhile || n.kind == NodeKind::LoopFor) ? IM_COL32(70, 30, 90, 255) : // Mor Donguler
                        IM_COL32(33, 38, 45, 255);

        drawList->AddRectFilled(pMin, ImVec2(pMax.x, pMin.y + 26.0f), headCol, 8.0f, ImDrawFlags_RoundCornersTop);
        drawList->AddText(ImVec2(pMin.x + 10, pMin.y + 6), IM_COL32(240, 246, 252, 255), n.title.c_str());

        // Icerik Ozeti
        if (n.kind == NodeKind::VariableDeclare) {
            std::string desc = TypeKindToString(n.varType) + std::string(" ") + n.targetVar + " = " + n.expr;
            drawList->AddText(ImVec2(pMin.x + 16, pMin.y + 40), IM_COL32(0, 200, 232, 255), desc.c_str());
        } else if (n.kind == NodeKind::Print) {
            drawList->AddText(ImVec2(pMin.x + 16, pMin.y + 40), IM_COL32(100, 220, 120, 255), ("cout << \"" + n.ioText + "\"").c_str());
        } else if (n.kind == NodeKind::Assign) {
            drawList->AddText(ImVec2(pMin.x + 16, pMin.y + 40), IM_COL32(220, 180, 80, 255), (n.targetVar + " = " + n.expr).c_str());
        } else if (n.kind == NodeKind::BranchIf) {
            drawList->AddText(ImVec2(pMin.x + 16, pMin.y + 40), IM_COL32(255, 170, 50, 255), ("Eger: " + n.conditionExpr).c_str());
        } else if (n.kind == NodeKind::LoopWhile) {
            drawList->AddText(ImVec2(pMin.x + 16, pMin.y + 40), IM_COL32(180, 120, 240, 255), ("While: " + n.conditionExpr).c_str());
        } else if (n.kind == NodeKind::LoopDoWhile) {
            drawList->AddText(ImVec2(pMin.x + 16, pMin.y + 40), IM_COL32(180, 120, 240, 255), ("Do-While: " + n.conditionExpr).c_str());
        } else if (n.kind == NodeKind::LoopFor) {
            drawList->AddText(ImVec2(pMin.x + 16, pMin.y + 40), IM_COL32(180, 120, 240, 255), ("For: " + n.loopInit + "; ..").c_str());
        }

        // Pinleri Ciz
        for (const auto& pin : n.pins) {
            ImVec2 pinPos(pMin.x + pin.relX, pMin.y + pin.relY);
            ImU32 pCol = (pin.type == PinType::ExecIn || pin.type == PinType::ExecOut) ? IM_COL32(0, 200, 232, 255) : IM_COL32(63, 185, 80, 255);
            
            if (pin.shape == PinShape::Diamond) {
                // Elmas / Kosul pini (True / False)
                float s = 5.0f;
                drawList->AddQuadFilled(
                    ImVec2(pinPos.x, pinPos.y - s),
                    ImVec2(pinPos.x + s, pinPos.y),
                    ImVec2(pinPos.x, pinPos.y + s),
                    ImVec2(pinPos.x - s, pinPos.y),
                    IM_COL32(255, 165, 0, 255)
                );
            } else if (pin.shape == PinShape::Triangle) {
                // Akis Ucgeni (Exec In / Out)
                float s = 5.0f;
                drawList->AddTriangleFilled(
                    ImVec2(pinPos.x - s, pinPos.y - s),
                    ImVec2(pinPos.x + s, pinPos.y),
                    ImVec2(pinPos.x - s, pinPos.y + s),
                    pCol
                );
            } else {
                // Daire
                drawList->AddCircleFilled(pinPos, 5.0f, pCol);
            }

            drawList->AddCircle(pinPos, 6.0f, IM_COL32(255, 255, 255, 150));

            if (pin.type == PinType::ExecIn) {
                drawList->AddText(ImVec2(pinPos.x + 8, pinPos.y - 7), IM_COL32(139, 148, 158, 255), pin.name.c_str());
            } else {
                drawList->AddText(ImVec2(pinPos.x - 70, pinPos.y - 7), IM_COL32(139, 148, 158, 255), pin.name.c_str());
            }
        }
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
        }
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Duzenlemek icin akistan bir dugume tiklayin.");
    }
}
