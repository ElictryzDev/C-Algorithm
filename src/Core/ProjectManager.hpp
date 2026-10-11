#pragma once

#include "Types.hpp"
#include <fstream>
#include <iostream>

class ProjectManager {
public:
    bool isProjectLoaded = false;
    std::string projectName = "Untitled_Project";
    std::string projectFilePath = "";
    std::string statusMessage = "Lutfen baslamak icin bir proje olusturun veya acin.";

    // Akış ve Değişken Durumu
    std::vector<VariableDefinition> variables;
    std::vector<FlowNode> nodes;
    std::vector<FlowWire> wires;
    int nextId = 1;

    // Danışman Durumu
    DataStructureType advisorDS = DataStructureType::Vector;
    OperationType advisorOp = OperationType::PopFront;
    int advisorCount = 10000;
    std::vector<DiagnosticMessage> advisorMessages;

    // Fonksiyonlar
    bool CreateNewProject(const std::string& name, const std::string& folderPath);
    bool SaveProject();
    bool OpenProject(const std::string& fullPath);
    void CloseProject();
    void SetupDefaultFlow();
    std::vector<std::string> GetSavedProjectFiles(const std::string& folderPath);

    // Düğüm ve Kablo Yardımcıları
    FlowNode* AddNode(NodeKind kind, const std::string& title, float x, float y);
    FlowNode* GetNode(int id);
    bool WouldCreateCycle(int fromNode, int toNode) const;
    bool ConnectPins(int fromNode, int fromPin, int toNode, int toPin);
    void DisconnectWire(int wireId);

    // Danışman Analizi (Birlikte yazdığımız kurallar)
    void RunDiagnostics();

    // C++ Kod Üreticisi
    std::string GenerateCppCode();
};
