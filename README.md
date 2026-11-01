# ⚡ C-Algorithm — Visual Scripting & Data Structure Engineering IDE

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/Language-C%2B%2B17-00599C.svg?logo=c%2B%2B)](https://isocpp.org/)
[![ANSI C (C99/C11)](https://img.shields.io/badge/Language-ANSI_C_(C99)-A8B9CC.svg?logo=c)](https://en.cppreference.com/w/c)
[![DirectX 11](https://img.shields.io/badge/Graphics-DirectX_11-green.svg)](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-graphics)
[![Dear ImGui](https://img.shields.io/badge/GUI-Dear_ImGui-orange.svg)](https://github.com/ocornut/imgui)
[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-Donate-yellow.svg)](https://buymeacoffee.com/elictryzn)

> **High-Performance Native Visual Algorithm & Data Structure Engineering IDE**  
> *Zero web overhead, 100% native C++17 / DirectX 11, 144+ FPS GPU-accelerated rendering.*  
> **Lead Architect & Author:** [Mustafa Can Yılmazer (@ElictryzDev)](https://github.com/ElictryzDev) — *Istanbul Beykent University, Computer Programming.*

---

## 🚀 Project Vision

**C-Algorithm** replaces obsolete, clunky academic flowchart tools (such as Flowgorithm) with modern game-engine node workflows (similar to Unreal Engine Blueprints) combined with international flowchart standards (ISO 5807).

Designed for computer science students and software engineers, C-Algorithm allows you to design algorithms visually while providing:
1. **Dual-Target Code Generation:** Seamlessly generate production-ready **Modern C++17** (`std::cout`, STL containers, RAII) or pure **ANSI C (C99/C11)** (`stdio.h`, `printf`, `scanf`, `typedef struct`, clean procedural memory).
2. **Real-Time Data Structure Advisor:** Live asymptotic time complexity ($O(1)$, $O(N)$, $O(N \log N)$) and physical memory layout analysis (Stack vs. Heap, cache locality, overflow risks).
3. **Resilient Input Stream Guard:** Built-in stream shields prevent infinite loops when non-numeric input is supplied to numeric `scanf` or `std::cin` operations.
4. **Zero-Dependency Portable Binary:** Statically linked (`/MT`) into a single **1.2 MB** executable (`exe/C-Algorithm.exe`). Runs instantly on any Windows 10/11 machine from a USB drive without installation or VC runtime DLL dependencies.

---

## 🌟 Key Features

### 🔒 Project Gating & `.calg` Format
* The canvas workspace remains securely locked until a project is created or opened.
* Projects are serialized and loaded via the lightweight `.calg` format.
* Automatic schema migration converts legacy tokens (e.g. `x < 10` loops) to modern strict bounds (`i <= 10`).

### ☕ Dual-Target Code Generator
* **⚡ Modern C++ (C++17):** Clean idiomatic C++, `std::vector`, `std::list`, `std::stack`, `std::queue`, Windows UTF-8 console output (`SetConsoleOutputCP(65001)`).
* **☕ Pure ANSI C (C99/C11):** Zero C++ overhead (no vtables, no iostream, no RTTI), raw libc memory footprints, `printf`/`scanf`, custom structs, and lightweight static memory helpers.
* **Dual Execution Modes:**
  * **In-App Terminal:** Runs with a 200-line safety limit against infinite loops.
  * **Interactive Command Prompt (CMD):** Spawns an external native Windows console with UTF-8 support for full keyboard input testing.

### 🧠 Data Structure Advisor
* Analyzes data structures in real-time (`std::vector`, `std::list`, `std::stack`, `std::queue`, `BinaryTree`).
* Flags CPU cache misses, dynamic heap fragmentation, and stack overflow vulnerabilities.

### 🎨 Visual Flowchart Canvas (ISO 5807 Compliant)
* **Pill / Oval:** `Program Start (main)`, `Program End (return 0)`, and `Loop End (})`.
* **Parallelogram:** `Console Output (printf / cout)` and `Keyboard Input (scanf / cin)`.
* **Diamond:** `Branch Condition (if - else)` with dedicated True (Green) and False (Red) execution pins.
* **Hexagon:** `For Loop`, `While Loop`, `Do-While Loop` with unified body outlet.
* **Rectangle:** `Variables`, `Multi-Variable Assignment`, `Math Operations`, and `Structs`.
* **Themes:** Default *Dark Amethyst Night* (Deep neon purple `#1A0D2E`, lilac grid `#461C78`, teal borders `#0D9488`, gold accents `#F59E0B`).

---

## 🛠️ How to Build & Run

### 1. Download Standalone Portable Executable
Grab the pre-compiled standalone binary directly from `exe/C-Algorithm.exe`. Double-click to launch on any Windows 10 or Windows 11 64-bit PC. No installer or runtime setup needed.

### 2. Build from Source (MSVC)
Prerequisites: **MSVC v143** (Visual Studio 2022 C++ Build Tools or Community Edition).

Open a command prompt in the project root and run:
```cmd
compile.bat
```
The script automatically builds with `/MT` (Static C Runtime) and outputs the packaged executable to `exe\C-Algorithm.exe`.

---

## ☕ Support & Sponsorship

C-Algorithm is an independent open-source engineering initiative created by Mustafa Can Yılmazer. If you find this tool helpful for your education or development, feel free to support:

* ☕ **Buy Me a Coffee:** [buymeacoffee.com/elictryzn](https://buymeacoffee.com/elictryzn)
* 🐙 **GitHub:** [@ElictryzDev](https://github.com/ElictryzDev)

---

## 📜 License & Intellectual Property

This project is open-source software licensed under the **[Apache License, Version 2.0](LICENSE)**.

### Mandatory Attribution Notice (Section 4):
Pursuant to Section 4 of the Apache License 2.0, any distribution, fork, modification, or inclusion of this software or its source code must retain all original copyright notices, patent statements, and the full attribution notice in the [NOTICE](NOTICE) file:

```text
C-Algorithm IDE
Copyright (C) 2026 Mustafa Can Yılmazer (ElictryzDev). All rights reserved.
Lead Developer: Mustafa Can Yılmazer
Academic Institution: Istanbul Beykent University - Computer Programming
```
For questions or collaborations, reach out via [GitHub](https://github.com/ElictryzDev) or [Buy Me a Coffee](https://buymeacoffee.com/elictryzn).
