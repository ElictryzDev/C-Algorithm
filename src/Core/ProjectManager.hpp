#pragma once

#include "Types.hpp"
#include <fstream>
#include <iostream>

enum class AppLanguage {
    Turkish,
    English
};

enum class AppTheme {
    OledDark,
    ClassicDark,
    Dracula,
    Cyberpunk,
    Light
};

class ProjectManager {
public:
    bool isProjectLoaded = false;
    std::string projectName = "Untitled_Project";
    std::string projectFilePath = "";
    std::string statusMessage = "Lutfen baslamak icin bir proje olusturun veya acin.";

    // Dil ve Tema Ayarlari
    AppLanguage currentLanguage = AppLanguage::Turkish;
    AppTheme currentTheme = AppTheme::OledDark;
    bool showGrid = true;

    // Akis ve Degisken Durumu
    std::vector<VariableDefinition> variables;
    std::vector<FlowNode> nodes;
    std::vector<FlowWire> wires;
    int nextId = 1;

    // Danisman Durumu
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

    // Dugum ve Kablo Yardimcilari
    FlowNode* AddNode(NodeKind kind, const std::string& title, float x, float y);
    FlowNode* GetNode(int id);
    bool DeleteNode(int id);
    bool WouldCreateCycle(int fromNode, int toNode) const;
    bool ConnectPins(int fromNode, int fromPin, int toNode, int toPin);
    void DisconnectWire(int wireId);

    // Danisman Analizi (Birlikte yazdigimiz kurallar)
    void RunDiagnostics();

    // C++ Kod Ureticisi
    std::string GenerateCppCode();
};
