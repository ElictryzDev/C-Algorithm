#pragma once

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <sstream>

// =========================================================================
// C-ALGORITHM ENGINE - CORE DATA & TYPE SYSTEM
// =========================================================================

enum class TypeKind {
    Int32,      // int
    Int64,      // long long
    Float,      // float
    Double,     // double
    Bool,       // bool
    UInt8,      // uint8_t  (0 - 255)
    UInt16,     // uint16_t (0 - 65,535)
    UInt32,     // uint32_t (0 - 4 Milyar)
    UInt64,     // uint64_t
    String      // std::string
};

inline const char* TypeKindToString(TypeKind t) {
    switch (t) {
        case TypeKind::Int32:  return "int";
        case TypeKind::Int64:  return "long long";
        case TypeKind::Float:  return "float";
        case TypeKind::Double: return "double";
        case TypeKind::Bool:   return "bool";
        case TypeKind::UInt8:  return "uint8_t";
        case TypeKind::UInt16: return "uint16_t";
        case TypeKind::UInt32: return "uint32_t";
        case TypeKind::UInt64: return "uint64_t";
        case TypeKind::String: return "std::string";
    }
    return "int";
}

inline size_t GetTypeByteSize(TypeKind t) {
    switch (t) {
        case TypeKind::UInt8:  return 1;
        case TypeKind::UInt16: return 2;
        case TypeKind::Int32:
        case TypeKind::UInt32:
        case TypeKind::Float:  return 4;
        case TypeKind::Int64:
        case TypeKind::UInt64:
        case TypeKind::Double: return 8;
        case TypeKind::Bool:   return 1;
        case TypeKind::String: return 32; // SSO buffer
    }
    return 4;
}

// ── Değişken Tanımı ──────────────────────────────────────────────────────
struct VariableDefinition {
    int id = 0;
    std::string name = "myVar";
    TypeKind type = TypeKind::Int32;
    std::string initialValue = "0";
};

// ── Düğüm ve Akış Tipleri ───────────────────────────────────────────────
enum class PinType {
    ExecIn,
    ExecOut,
    DataIn,
    DataOut
};

enum class NodeKind {
    Start,              // Program Başlangıcı (main())
    End,                // Program Sonu (return 0)
    VariableDeclare,    // Değişken Tanımla (örn: uint8_t can = 100)
    Assign,             // Değer Ata (örn: x = x + 10)
    Print,              // Ekrana Yaz (std::cout << ...)
    Input,              // Girdi Al (std::cin >> ...)
    BranchIf,           // Kosul (if/else)
    LoopWhile,          // While Dongusu
    LoopDoWhile,        // Do-While Dongusu
    LoopFor,            // For Dongusu
    SwitchCase,         // Switch-Case Coklu Karar Yapisi
    StructDef,          // Struct Tanimlama Blogu
    VectorOp,           // std::vector Islemleri (push_back, pop_back vb.)
    ListOp,             // std::list Islemleri (push_front, push_back, pop_front vb.)
    StackOp,            // std::stack Islemleri (push, pop, top - LIFO)
    QueueOp,            // std::queue Islemleri (push, pop, front - FIFO)
    ArrayOp,            // Sabit Boyutlu Dizi (std::array / C-Dizi)
    MathOp              // Matematiksel Islem (+, -, *, /, %, sqrt, pow, fact)
};

enum class PointerKind {
    RawValue,       // Normal Deger (Stack)
    SharedPtr,      // std::shared_ptr<T> (Heap + RAII Reference Counting)
    UniquePtr       // std::unique_ptr<T> (Heap + Tekil Sahiplik)
};

struct StructMember {
    std::string name = "alan";
    TypeKind type = TypeKind::Int32;
    std::string defaultValue = "0";
    bool hasDefault = true;
};

enum class PinShape {
    Triangle,   // Akis / Exec pini (Beyaz Ucgen)
    Circle,     // Veri pini (Daire)
    Diamond     // Kosul / Boolean pini (Elmas / Sekizgen)
};

struct FlowPin {
    int id = 0;
    std::string name;
    PinType type = PinType::ExecIn;
    PinShape shape = PinShape::Triangle;
    float relX = 0.0f;
    float relY = 0.0f;
};

struct AssignmentEntry {
    std::string varName = "x";
    TypeKind varType = TypeKind::Int32;
    std::string valueExpr = "0";
};

struct FlowNode {
    int id = 0;
    NodeKind kind = NodeKind::Start;
    std::string title = "Node";
    
    // Degisken ve Atama Ayarlari
    std::string targetVar = "x";
    TypeKind varType = TypeKind::Int32;
    std::string expr = "0";
    bool isConst = false; // const degismez tanimi
    
    // Akilli Isaretci & Ozel Tip Ayarlari
    PointerKind pointerKind = PointerKind::RawValue;
    bool isCustomType = false;
    std::string customTypeName = "Oyuncu";

    // Coklu Degisken / Coklu Atama Soketleri
    std::vector<AssignmentEntry> multiAssignments;

    // Girdi / Cikti Ayarlari
    std::string ioText = "Merhaba C-Algorithm!";
    int printMode = 0; // 0: Duz Metin, 1: Degisken Degeri, 2: Metin + Degisken

    // Matematiksel Islem Ayarlari (MathOp)
    std::string mathTarget = "sonuc";
    std::string mathLeft = "a";
    std::string mathOp = "+"; // "+", "-", "*", "/", "%", "sqrt", "pow", "fact"
    std::string mathRight = "b";

    // Kosul ve Dongu Ayarlari
    std::string conditionExpr = "x < 10";  // If / While / Do-While / For kosulu
    std::string loopInit = "int i = 0";    // For baslangic
    std::string loopStep = "i++";          // For artis
    // While / DoWhile govde ek satirlari (sayac++, x *= 2, vb.)
    std::vector<std::string> loopBodyStmts;  // Her eleman gövdeye ayrı satır olarak yazılır

    // Switch-Case Ayarlari
    std::string switchVar = "secim";
    std::vector<std::string> caseValues = { "1", "2" }; // Case durumlari

    // Struct Tanimlama Ayarlari
    std::string structName = "Oyuncu";
    std::vector<StructMember> structMembers;
    bool hasDestructor = true; // RAII Yikici Metot (~Oyuncu())
    std::string destructorMsg = "Oyuncu nesnesi bellekten silindi (Destructor cagirildi)!";

    // Veri Yapilari Ayarlari (vector, list, stack, queue, array)
    std::string dsName = "veriYapisi";
    std::string dsOpType = "push_back";
    std::string dsOpArg = "10";
    int arraySize = 5;
    int arrayIndex = 0;

    // std::vector Veri Yapisi Ayarlari (Geriye donuk uyumluluk)
    std::string vectorName = "sayilar";
    std::string vectorOpType = "push_back"; // "push_back", "pop_back", "clear"
    std::string vectorOpArg = "10";

    float x = 80.0f;
    float y = 80.0f;
    float width = 220.0f;
    float height = 110.0f;
    std::vector<FlowPin> pins;

    // Hata ve Uyari Durumu (Gorsel Linter)
    bool hasWarning = false;
    std::string warningText;
};

struct FlowWire {
    int id = 0;
    int fromNode = -1;
    int fromPin = -1;
    int toNode = -1;
    int toPin = -1;
};

// ── Veri Yapısı Danışmanı Tipleri ────────────────────────────────────────
enum class DataStructureType {
    Vector,
    List,
    Stack,
    Queue,
    BinaryTree
};

enum class OperationType {
    PushBack,
    PopFront,
    InsertMiddle,
    RandomAccess,
    Search
};

struct DiagnosticMessage {
    enum Level { Info, Warning, Error } level = Warning;
    std::string title;
    std::string diagnosis;
    std::string explanation;
    std::string prescription;
    std::string timeComplexity;
};
