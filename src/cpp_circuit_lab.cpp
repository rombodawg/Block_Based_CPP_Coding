#include "core.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <windowsx.h>
#include <regex>

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")

namespace {
using cpplab::Block;
using cpplab::BlockInfo;
using cpplab::BlockType;

const wchar_t* APP_CLASS = L"CppCircuitLabWindow";
const wchar_t* PROMPT_CLASS = L"CppCircuitLabPrompt";

COLORREF C_BG = RGB(17, 21, 29);
COLORREF C_PANEL = RGB(27, 33, 44);
COLORREF C_PANEL2 = RGB(34, 41, 54);
COLORREF C_EDGE = RGB(62, 72, 89);
COLORREF C_TEXT = RGB(234, 239, 247);
COLORREF C_MUTED = RGB(158, 171, 190);
COLORREF C_GOLD = RGB(243, 184, 82);
COLORREF C_GREEN = RGB(91, 190, 132);
COLORREF C_RED = RGB(233, 103, 103);
COLORREF C_BLUE = RGB(92, 159, 224);
COLORREF C_PURPLE = RGB(177, 126, 213);
COLORREF C_TEAL = RGB(68, 177, 177);
COLORREF C_ORANGE = RGB(220, 140, 66);

struct Rect { int x = 0, y = 0, w = 0, h = 0; };
constexpr int GRID_CELL = 48;              // One logical grid cell in the program canvas.
constexpr int CANVAS_PITCH = GRID_CELL;      // One block per row; each row is one grid cell tall.
constexpr int TILE_SIZE = 42;                // Square front face with a small 3D bevel.
constexpr int TILE_INSET = (GRID_CELL - TILE_SIZE) / 2;
constexpr int GRID_ORIGIN_INSET = 3;
constexpr int BLOCK_ORIGIN_INSET = GRID_ORIGIN_INSET + TILE_INSET;
constexpr int PALETTE_COLUMNS = 3;
constexpr int PALETTE_ROW_HEIGHT = 50;
constexpr int PALETTE_COLUMN_PITCH = 52;
constexpr int GLOSSARY_ROW_HEIGHT = 180;
static_assert(CANVAS_PITCH == GRID_CELL, "Each block must occupy exactly one grid row.");
static_assert(TILE_SIZE > 0 && TILE_SIZE < GRID_CELL, "Square tiles must fit inside one grid cell.");
bool contains(const Rect& r, int x, int y) { return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h; }
RECT native_rect(const Rect& r) { return RECT{r.x, r.y, r.x + r.w, r.y + r.h}; }
void fill_rect(HDC dc, Rect r, COLORREF c);

std::wstring to_wide(const std::string& input) {
    if (input.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), nullptr, 0);
    if (size <= 0) return L"";
    std::wstring out(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), out.data(), size);
    return out;
}
std::string to_utf8(const std::wstring& input) {
    if (input.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) return "";
    std::string out(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), out.data(), size, nullptr, nullptr);
    return out;
}
std::string ansi_to_utf8(const std::string& bytes) {
    if (bytes.empty()) return {};
    int wsize = MultiByteToWideChar(CP_OEMCP, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    if (wsize <= 0) wsize = MultiByteToWideChar(CP_ACP, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    if (wsize <= 0) return bytes;
    std::wstring w(static_cast<size_t>(wsize), L'\0');
    if (!MultiByteToWideChar(CP_OEMCP, 0, bytes.data(), static_cast<int>(bytes.size()), w.data(), wsize))
        MultiByteToWideChar(CP_ACP, 0, bytes.data(), static_cast<int>(bytes.size()), w.data(), wsize);
    return to_utf8(w);
}

std::wstring quote_arg(const std::wstring& arg) {
    std::wstring out = L"\"";
    size_t slashes = 0;
    for (wchar_t ch : arg) {
        if (ch == L'\\') { ++slashes; continue; }
        if (ch == L'"') {
            out.append(slashes * 2 + 1, L'\\');
            out += ch;
            slashes = 0;
            continue;
        }
        out.append(slashes, L'\\');
        slashes = 0;
        out += ch;
    }
    out.append(slashes * 2, L'\\');
    out += L'"';
    return out;
}

COLORREF block_color(BlockType t) {
    switch (t) {
        case BlockType::IncludeHeader: return RGB(93, 68, 128);
        case BlockType::PrintText: return RGB(145, 76, 31);
        case BlockType::PrintExpression: return RGB(43, 111, 69);
        case BlockType::DeclareInt: return RGB(42, 83, 139);
        case BlockType::DeclareDouble: return RGB(37, 99, 128);
        case BlockType::DeclareText: return RGB(100, 63, 133);
        case BlockType::DeclareBool: return RGB(37, 111, 102);
        case BlockType::DeclareChar: return RGB(130, 65, 104);
        case BlockType::DeclareIntList: return RGB(38, 93, 128);
        case BlockType::ShowVariable: return RGB(43, 91, 148);
        case BlockType::SetVariable: return RGB(45, 80, 146);
        case BlockType::ReadInput: return RGB(43, 99, 126);
        case BlockType::If: return RGB(35, 105, 103);
        case BlockType::ElseIf: return RGB(38, 111, 113);
        case BlockType::Else: return RGB(40, 91, 101);
        case BlockType::Repeat: return RGB(133, 94, 30);
        case BlockType::ForLoop: return RGB(143, 91, 29);
        case BlockType::While: return RGB(140, 73, 31);
        case BlockType::Break: return RGB(136, 64, 61);
        case BlockType::Continue: return RGB(43, 103, 106);
        case BlockType::Return: return RGB(95, 73, 125);
        case BlockType::RawStatement: return RGB(76, 79, 139);
        case BlockType::RawProgram: return RGB(102, 59, 123);
        case BlockType::Comment: case BlockType::ActionComment: return RGB(79, 91, 108);
        case BlockType::ActionSetNumber: case BlockType::ActionSetText: case BlockType::ActionChangeNumber: return RGB(161, 111, 42);
        case BlockType::ActionPrintText: case BlockType::ActionShowValue: case BlockType::ActionReadNumber: return RGB(41, 113, 80);
        case BlockType::TokIf: case BlockType::TokElse: case BlockType::TokFor: case BlockType::TokWhile:
        case BlockType::TokDo: case BlockType::TokSwitch: case BlockType::TokCase: case BlockType::TokDefault:
        case BlockType::TokBreak: case BlockType::TokContinue: case BlockType::TokReturn: return RGB(39, 120, 116);
        case BlockType::TokLParen: case BlockType::TokRParen: case BlockType::TokLBrace: case BlockType::TokRBrace:
        case BlockType::TokLBracket: case BlockType::TokRBracket: case BlockType::TokSemicolon: case BlockType::TokComma:
        case BlockType::TokColon: case BlockType::TokQuestion: return RGB(150, 98, 39);
        case BlockType::TokAssign: case BlockType::TokPlus: case BlockType::TokMinus: case BlockType::TokStar:
        case BlockType::TokSlash: case BlockType::TokPercent: case BlockType::TokEqual: case BlockType::TokNotEqual:
        case BlockType::TokLess: case BlockType::TokGreater: case BlockType::TokLessEqual: case BlockType::TokGreaterEqual:
        case BlockType::TokAnd: case BlockType::TokOr: case BlockType::TokNot: case BlockType::TokShiftLeft:
        case BlockType::TokShiftRight: case BlockType::TokPlusAssign: case BlockType::TokMinusAssign:
        case BlockType::TokMultiplyAssign: case BlockType::TokDivideAssign: case BlockType::TokModAssign: return RGB(50, 100, 164);
        case BlockType::TokIdentifier: case BlockType::TokTypeName: case BlockType::TokExpression:
        case BlockType::TokNumber: case BlockType::TokStringLiteral: case BlockType::TokCharLiteral: return RGB(112, 76, 156);
        case BlockType::TokInclude: case BlockType::TokInt: case BlockType::TokDouble: case BlockType::TokFloat:
        case BlockType::TokString: case BlockType::TokBool: case BlockType::TokChar: case BlockType::TokVoid:
        case BlockType::TokAuto: case BlockType::TokConst: case BlockType::TokClass: case BlockType::TokStruct:
        case BlockType::TokTemplate: case BlockType::TokTypename: case BlockType::TokNamespace: case BlockType::TokUsing:
        case BlockType::TokTry: case BlockType::TokCatch: case BlockType::TokThrow: case BlockType::TokStatic:
        case BlockType::TokVirtual: case BlockType::TokOverride: case BlockType::TokSizeof: case BlockType::TokMain:
        case BlockType::TokStd: case BlockType::TokCout: case BlockType::TokCin: case BlockType::TokEndl:
        case BlockType::TokVector: case BlockType::TokTrue: case BlockType::TokFalse: case BlockType::TokNullptr:
        case BlockType::TokKeyword: case BlockType::TokHash: case BlockType::TokSymbol: return RGB(58, 99, 151);
    }
    return C_BLUE;
}

COLORREF brighten(COLORREF c, int amount) {
    auto channel = [amount](int v) { return std::min(255, v + amount); };
    return RGB(channel(GetRValue(c)), channel(GetGValue(c)), channel(GetBValue(c)));
}

COLORREF darken(COLORREF c, int amount) {
    auto channel = [amount](int v) { return std::max(0, v - amount); };
    return RGB(channel(GetRValue(c)), channel(GetGValue(c)), channel(GetBValue(c)));
}

void fill_polygon(HDC dc, const POINT* points, int count, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
    Polygon(dc, const_cast<POINT*>(points), count);
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(brush);
}

// A block is a square tile: equal width and height, sharp square silhouette, and beveled
// top/left highlights plus right/bottom shadows. There are deliberately no puzzle tabs.
void draw_cube_tile(HDC dc, Rect r, COLORREF fill, COLORREF border, int borderWidth = 1) {
    const int bevel = 5;
    fill_rect(dc, {r.x + 2, r.y + 3, r.w, r.h}, RGB(9, 12, 18)); // cast shadow
    fill_rect(dc, r, fill);

    POINT topFace[] = {
        {r.x + 1, r.y + 1}, {r.x + r.w - bevel, r.y + 1},
        {r.x + r.w - 1, r.y + bevel}, {r.x + 1, r.y + bevel}
    };
    fill_polygon(dc, topFace, static_cast<int>(std::size(topFace)), brighten(fill, 46));

    POINT leftFace[] = {
        {r.x + 1, r.y + 1}, {r.x + bevel, r.y + bevel},
        {r.x + bevel, r.y + r.h - bevel}, {r.x + 1, r.y + r.h - 1}
    };
    fill_polygon(dc, leftFace, static_cast<int>(std::size(leftFace)), brighten(fill, 24));

    POINT rightFace[] = {
        {r.x + r.w - bevel, r.y + 1}, {r.x + r.w - 1, r.y + bevel},
        {r.x + r.w - 1, r.y + r.h - 1}, {r.x + r.w - bevel, r.y + r.h - bevel}
    };
    fill_polygon(dc, rightFace, static_cast<int>(std::size(rightFace)), darken(fill, 50));

    POINT bottomFace[] = {
        {r.x + 1, r.y + r.h - bevel}, {r.x + r.w - bevel, r.y + r.h - bevel},
        {r.x + r.w - 1, r.y + r.h - 1}, {r.x + 1, r.y + r.h - 1}
    };
    fill_polygon(dc, bottomFace, static_cast<int>(std::size(bottomFace)), darken(fill, 33));

    HPEN pen = CreatePen(PS_SOLID, borderWidth, border);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
    Rectangle(dc, r.x, r.y, r.x + r.w, r.y + r.h);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(pen);
}


std::string shorten(const std::string& text, size_t max);

std::string tile_face_label(const Block& b) {
    switch (b.type) {
        case BlockType::TokExpression:
            return "expr";
        case BlockType::TokIdentifier: case BlockType::TokTypeName:
        case BlockType::TokKeyword: case BlockType::TokNumber:
            return shorten(b.value, 5);
        case BlockType::TokStringLiteral: {
            std::string preview = b.value;
            auto replace_all = [&](const std::string& from, const std::string& to) {
                size_t p = 0; while ((p = preview.find(from, p)) != std::string::npos) { preview.replace(p, from.size(), to); p += to.size(); }
            };
            replace_all("\n", "\\n"); replace_all("\r", "\\r"); replace_all("\t", "\\t");
            return preview.size() > 3 ? std::string("\"") + preview.substr(0, 2) + "…\"" : std::string("\"") + preview + "\"";
        }
        case BlockType::TokCharLiteral:
            return b.value == "\\n" ? "'\\n'" : std::string("'") + shorten(b.value, 2) + "'";
        case BlockType::TokSymbol: return shorten(b.value, 5);
        default: return cpplab::tile_label(b.type);
    }
}

std::string block_summary(const Block& b) {
    if (cpplab::is_atomic_action_type(b.type)) return cpplab::action_source(b);
    if (cpplab::is_atomic_mode_type(b.type)) return cpplab::atomic_token_text(b);
    switch (b.type) {
        case BlockType::IncludeHeader: return cpplab::normalize_header(b.value);
        case BlockType::PrintText: return "\"" + b.value + "\"";
        case BlockType::PrintExpression: return b.value;
        case BlockType::DeclareInt: return "int " + b.value + " = " + b.extra;
        case BlockType::DeclareDouble: return "double " + b.value + " = " + b.extra;
        case BlockType::DeclareText: return "string " + b.value + " = \"" + b.extra + "\"";
        case BlockType::DeclareBool: return "bool " + b.value + " = " + b.extra;
        case BlockType::DeclareChar: return "char " + b.value + " = '" + b.extra + "'";
        case BlockType::DeclareIntList: return "" + std::string(b.value) + " = {" + b.extra + "}";
        case BlockType::ShowVariable: return b.value;
        case BlockType::SetVariable: return b.value + " = " + b.extra;
        case BlockType::ReadInput: return "cin >> " + b.value;
        case BlockType::If: return b.value;
        case BlockType::ElseIf: return b.value;
        case BlockType::Else: return "all earlier tests were false";
        case BlockType::Repeat: return b.value + " times";
        case BlockType::ForLoop: return b.value;
        case BlockType::While: return b.value;
        case BlockType::Break: return "exit this loop";
        case BlockType::Continue: return "skip to next pass";
        case BlockType::Return: return "return " + b.value;
        case BlockType::RawStatement: return b.value.size() <= 80 ? b.value : b.value.substr(0, 77) + "...";
        case BlockType::RawProgram: return "complete C++ source file";
        case BlockType::Comment: return b.value;
    }
    return {};
}

std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    if (lines.empty()) lines.emplace_back();
    return lines;
}
std::string shorten(const std::string& text, size_t max) {
    if (text.size() <= max) return text;
    if (max < 4) return text.substr(0, max);
    return text.substr(0, max - 3) + "...";
}

struct PromptData {
    HWND hwnd = nullptr;
    HWND edit = nullptr;
    std::wstring title;
    std::wstring prompt;
    std::wstring initial;
    std::wstring result;
    bool done = false;
    bool accepted = false;
    bool multiline = false;
};

LRESULT CALLBACK prompt_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* data = reinterpret_cast<PromptData*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        data = static_cast<PromptData*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
    }
    switch (msg) {
        case WM_CREATE: {
            data = reinterpret_cast<PromptData*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            const int contentW = data->multiline ? 700 : 450;
            const int editH = data->multiline ? 340 : 28;
            const int buttonY = data->multiline ? 424 : 96;
            CreateWindowExW(0, L"STATIC", data->prompt.c_str(), WS_CHILD | WS_VISIBLE,
                18, 16, contentW, 34, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            DWORD editStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
            editStyle |= data->multiline ? (ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL) : ES_AUTOHSCROLL;
            data->edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", data->initial.c_str(),
                editStyle, 18, 53, contentW, editH,
                hwnd, reinterpret_cast<HMENU>(1001), GetModuleHandleW(nullptr), nullptr);
            SendMessageW(data->edit, EM_SETLIMITTEXT, data->multiline ? 30000 : 4000, 0);
            CreateWindowExW(0, L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                data->multiline ? 526 : 294, buttonY, 82, 29, hwnd, reinterpret_cast<HMENU>(IDOK), GetModuleHandleW(nullptr), nullptr);
            CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                data->multiline ? 618 : 386, buttonY, 82, 29, hwnd, reinterpret_cast<HMENU>(IDCANCEL), GetModuleHandleW(nullptr), nullptr);
            SendMessageW(data->edit, EM_SETSEL, 0, -1);
            SetFocus(data->edit);
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wp) == IDOK) {
                int length = GetWindowTextLengthW(data->edit);
                std::wstring value(static_cast<size_t>(length) + 1, L'\0');
                if (length) GetWindowTextW(data->edit, value.data(), length + 1);
                value.resize(static_cast<size_t>(length));
                data->result = value;
                data->accepted = true;
                data->done = true;
                DestroyWindow(hwnd);
                return 0;
            }
            if (LOWORD(wp) == IDCANCEL) {
                data->accepted = false;
                data->done = true;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        case WM_CLOSE:
            data->accepted = false;
            data->done = true;
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

bool prompt_input(HWND owner, const wchar_t* title, const wchar_t* prompt,
                  const std::string& initial, std::string& result, bool multiline = false) {
    PromptData data;
    data.title = title;
    data.prompt = prompt;
    data.initial = to_wide(initial);
    data.multiline = multiline;
    const int dialogW = multiline ? 760 : 500;
    const int dialogH = multiline ? 500 : 165;
    RECT ownerRect{};
    GetWindowRect(owner, &ownerRect);
    int x = ownerRect.left + ((ownerRect.right - ownerRect.left) - dialogW) / 2;
    int y = ownerRect.top + ((ownerRect.bottom - ownerRect.top) - dialogH) / 2;
    EnableWindow(owner, FALSE);
    data.hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, PROMPT_CLASS,
        data.title.c_str(), WS_CAPTION | WS_SYSMENU | WS_POPUP,
        x, y, dialogW, dialogH, owner, nullptr, GetModuleHandleW(nullptr), &data);
    if (!data.hwnd) {
        EnableWindow(owner, TRUE);
        return false;
    }
    ShowWindow(data.hwnd, SW_SHOW);
    UpdateWindow(data.hwnd);
    while (!data.done) {
        MSG msg{};
        BOOL got = GetMessageW(&msg, nullptr, 0, 0);
        if (got <= 0) break;
        if (!IsDialogMessageW(data.hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    if (IsWindow(data.hwnd)) DestroyWindow(data.hwnd);
    EnableWindow(owner, TRUE);
    SetForegroundWindow(owner);
    if (data.accepted) result = to_utf8(data.result);
    return data.accepted;
}

struct Compiler {
    std::wstring path;
    enum class Kind { Gpp, Clang, Msvc } kind = Kind::Gpp;
    std::string name;
};

std::wstring find_executable(const wchar_t* name) {
    wchar_t path[MAX_PATH * 4]{};
    DWORD n = SearchPathW(nullptr, name, nullptr, static_cast<DWORD>(std::size(path)), path, nullptr);
    if (!n || n >= std::size(path)) return {};
    return path;
}
void prepend_directory_to_process_path(const std::wstring& directory) {
    // Keep MinGW's runtime DLLs discoverable by both the compiler and the
    // user-created .exe files that it launches.
    DWORD needed = GetEnvironmentVariableW(L"PATH", nullptr, 0);
    std::wstring existing;
    if (needed > 0) {
        std::vector<wchar_t> buffer(static_cast<size_t>(needed) + 1, L'\0');
        DWORD copied = GetEnvironmentVariableW(L"PATH", buffer.data(), static_cast<DWORD>(buffer.size()));
        if (copied > 0 && copied < buffer.size()) existing.assign(buffer.data(), copied);
    }
    std::wstring updated = directory;
    if (!existing.empty()) updated += L";" + existing;
    SetEnvironmentVariableW(L"PATH", updated.c_str());
}

Compiler find_compiler() {
    // Launch scripts can supply an installation root without embedding any user's path.
    DWORD rootSize = GetEnvironmentVariableW(L"CPP_CIRCUIT_LAB_MSYS2_ROOT", nullptr, 0);
    if (rootSize > 0 && rootSize < 32768) {
        std::vector<wchar_t> rootBuffer(static_cast<size_t>(rootSize) + 1, L'\0');
        DWORD copied = GetEnvironmentVariableW(L"CPP_CIRCUIT_LAB_MSYS2_ROOT", rootBuffer.data(), static_cast<DWORD>(rootBuffer.size()));
        if (copied > 0 && copied < rootBuffer.size()) {
            std::wstring root(rootBuffer.data(), copied);
            while (!root.empty() && (root.back() == L'\\' || root.back() == L'/')) root.pop_back();
            const std::wstring ucrt64Bin = root + L"\\ucrt64\\bin";
            const std::wstring ucrt64Gpp = ucrt64Bin + L"\\g++.exe";
            DWORD attrs = GetFileAttributesW(ucrt64Gpp.c_str());
            if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                prepend_directory_to_process_path(ucrt64Bin);
                return {ucrt64Gpp, Compiler::Kind::Gpp, "MSYS2 UCRT64 g++"};
            }
        }
    }

    auto gpp = find_executable(L"g++.exe");
    if (!gpp.empty()) return {gpp, Compiler::Kind::Gpp, "g++ (MinGW/MSYS2)"};
    auto clang = find_executable(L"clang++.exe");
    if (!clang.empty()) return {clang, Compiler::Kind::Clang, "clang++"};
    auto cl = find_executable(L"cl.exe");
    if (!cl.empty()) return {cl, Compiler::Kind::Msvc, "MSVC cl.exe"};
    return {};
}

struct ProcessResult { DWORD exitCode = 0; bool timedOut = false; std::string output; };
ProcessResult run_process(const std::wstring& executable, std::wstring commandLine, DWORD timeoutMs, const std::string& standardInput = {}) {
    ProcessResult result;
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    HANDLE readPipe = nullptr, writePipe = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
        result.exitCode = 1;
        result.output = "Could not create an output pipe for the compiler. Restart the app and try again.";
        return result;
    }
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);
    HANDLE inputReadPipe = nullptr, inputWritePipe = nullptr;
    HANDLE nul = INVALID_HANDLE_VALUE;
    if (!standardInput.empty()) {
        if (!CreatePipe(&inputReadPipe, &inputWritePipe, &sa, 0)) {
            CloseHandle(readPipe); CloseHandle(writePipe);
            result.exitCode = 1;
            result.output = "Could not prepare the program input pipe. Try running again.";
            return result;
        }
        SetHandleInformation(inputWritePipe, HANDLE_FLAG_INHERIT, 0);
    } else {
        nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    }
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = inputReadPipe ? inputReadPipe : (nul == INVALID_HANDLE_VALUE ? GetStdHandle(STD_INPUT_HANDLE) : nul);
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;
    PROCESS_INFORMATION pi{};
    std::wstring mutableCommand = quote_arg(executable) + L" " + commandLine;
    std::vector<wchar_t> command(mutableCommand.begin(), mutableCommand.end());
    command.push_back(L'\0');
    BOOL created = CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &si, &pi);
    CloseHandle(writePipe);
    if (inputReadPipe) CloseHandle(inputReadPipe);
    if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
    if (!created) {
        if (inputWritePipe) CloseHandle(inputWritePipe);
        CloseHandle(readPipe);
        result.exitCode = 1;
        result.output = "Windows could not start the compiler or program. Check that the compiler is installed and on PATH.";
        return result;
    }
    if (inputWritePipe) {
        size_t inputOffset = 0;
        while (inputOffset < standardInput.size()) {
            DWORD written = 0;
            const DWORD request = static_cast<DWORD>(std::min<size_t>(standardInput.size() - inputOffset, 32768));
            if (!WriteFile(inputWritePipe, standardInput.data() + inputOffset, request, &written, nullptr) || written == 0) break;
            inputOffset += written;
        }
        CloseHandle(inputWritePipe); // EOF tells std::cin there is no more input.
    }

    const auto start = std::chrono::steady_clock::now();
    bool processDone = false;
    while (!processDone) {
        DWORD available = 0;
        while (PeekNamedPipe(readPipe, nullptr, 0, nullptr, &available, nullptr) && available > 0) {
            char buffer[4096];
            DWORD read = 0;
            if (!ReadFile(readPipe, buffer, static_cast<DWORD>(std::min<size_t>(sizeof(buffer), available)), &read, nullptr) || read == 0) break;
            result.output.append(buffer, buffer + read);
            if (result.output.size() > 1024 * 1024) {
                result.output += "\n[Output clipped at 1 MB to keep the learning app responsive.]\n";
                TerminateProcess(pi.hProcess, 2);
                break;
            }
        }
        DWORD wait = WaitForSingleObject(pi.hProcess, 15);
        if (wait == WAIT_OBJECT_0) processDone = true;
        else if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(timeoutMs)) {
            result.timedOut = true;
            TerminateProcess(pi.hProcess, 124);
            WaitForSingleObject(pi.hProcess, 500);
            processDone = true;
        }
    }
    // Drain any bytes written immediately before process exit.
    for (;;) {
        DWORD available = 0;
        if (!PeekNamedPipe(readPipe, nullptr, 0, nullptr, &available, nullptr) || available == 0) break;
        char buffer[4096]; DWORD read = 0;
        if (!ReadFile(readPipe, buffer, static_cast<DWORD>(std::min<size_t>(sizeof(buffer), available)), &read, nullptr) || read == 0) break;
        result.output.append(buffer, buffer + read);
        if (result.output.size() > 1024 * 1024) break;
    }
    GetExitCodeProcess(pi.hProcess, &result.exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(readPipe);
    result.output = ansi_to_utf8(result.output);
    if (result.timedOut) result.output += "\nStopped because the program took too long. Check whether a While condition can ever become false.\n";
    return result;
}

std::filesystem::path working_directory() {
    wchar_t temp[MAX_PATH * 4]{};
    DWORD n = GetTempPathW(static_cast<DWORD>(std::size(temp)), temp);
    std::filesystem::path base = n && n < std::size(temp) ? std::filesystem::path(temp) : std::filesystem::temp_directory_path();
    auto dir = base / L"CppCircuitLab";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir;
}

struct Hit;
std::vector<std::string> wrap_output(const std::string& text, int chars);

bool atomic_canvas_mode(const std::vector<Block>& blocks) {
    return std::any_of(blocks.begin(), blocks.end(), [](const Block& b) { return cpplab::is_atomic_mode_type(b.type); });
}
int canvas_content_height(const std::vector<Block>& blocks) {
    if (!atomic_canvas_mode(blocks)) return static_cast<int>(blocks.size()) * CANVAS_PITCH + 16;
    int maxRow = -1;
    for (const auto& b : blocks) maxRow = std::max(maxRow, b.gridY);
    return (maxRow + 1) * GRID_CELL + 16;
}
int canvas_max_grid_row(const std::vector<Block>& blocks) {
    if (!atomic_canvas_mode(blocks)) return static_cast<int>(blocks.size());
    int maxRow = -1;
    for (const auto& b : blocks) maxRow = std::max(maxRow, b.gridY);
    return maxRow + 1;
}
int canvas_max_grid_column(const std::vector<Block>& blocks) {
    int maxColumn = -1;
    if (atomic_canvas_mode(blocks)) {
        for (const auto& b : blocks) maxColumn = std::max(maxColumn, b.gridX);
    }
    return maxColumn + 1;
}

struct UndoState {
    std::vector<Block> blocks;
    std::string importedSource;
    bool importedSourceActive = false;
};

struct App {
    HWND hwnd = nullptr;
    HFONT font = nullptr, smallFont = nullptr, monoFont = nullptr, titleFont = nullptr, tileFont = nullptr;
    std::vector<Block> blocks = cpplab::atomic_example_program();
    std::vector<UndoState> undoStack;
    int selected = -1;
    std::string importedSource;
    bool importedSourceActive = false;
    ULONGLONG clearAllArmedUntil = 0;
    std::vector<std::string> codeLines;
    std::vector<int> codeOwners;
    std::string output = "Welcome to C++ Circuit Lab.\n\nChoose a block, change it, predict the result, then press Run.\n\nYour blocks generate real C++. Your mistakes are part of the lesson.";
    std::string status = "Hand: empty. Click a library tile to pick it up, then click a grid cell to place it. Press Esc to drop it.";
    std::string compilerStatus;
    std::vector<Rect> blockRects;
    std::vector<Rect> paletteRects;
    int paletteScroll = 0;
    int canvasScroll = 0;
    int canvasScrollX = 0;
    int codeScroll = 0;
    int outputScroll = 0;
    int glossaryScroll = 0;
    bool overlayGlossary = false;
    bool overlayHelp = false;
    bool showHelp = false;
    std::vector<struct Hit> hits;
    enum class ScrollKind { None, Palette, Canvas, Code, Output, Glossary } scrollDrag = ScrollKind::None;
    int scrollGrabY = 0;
    int dragSource = -1;
    int paletteDragType = -1;
    int paletteDragStartX = 0, paletteDragStartY = 0;
    bool paletteDragKeep = false;
    int handBlockType = -1;
    bool handSticky = false;
    int paletteDragGrabX = 0, paletteDragGrabY = 0;
    int dragStartY = 0;
    int dragStartX = 0;
    int dragGrabX = 0;
    int dragGrabY = 0;
    int mouseX = 0, mouseY = 0;
    int hoveredPalette = -1;
    int hoveredCanvas = -1;
    std::filesystem::path tempDir;
    Compiler compiler;
    cpplab::GenerateResult generated;
    int lastRunBlock = -1;

    App() {
        tempDir = working_directory();
        compiler = find_compiler();
        compilerStatus = compiler.path.empty() ? "No C++ compiler found" : "Compiler: " + compiler.name;
        refresh_code();
        if (!compiler.path.empty()) {
            output = "Welcome to C++ Circuit Lab.\n\nCompiler selected for every Run:\n" + to_utf8(compiler.path) +
                "\n\nPress Run to build your blocks into a C++ program, then launch it. The Output & Feedback panel will show the compiler path and each build/run stage.";
        }
        if (compiler.path.empty()) {
            output = "Welcome to C++ Circuit Lab.\n\nThe visual editor works without a compiler, but Run needs a C++ compiler.\n\nRun Start_Cpp_Circuit_Lab.bat and enter your MSYS2 installation folder when prompted, or make a C++ compiler available on PATH. You can also launch from a Visual Studio Developer Command Prompt.\n\nYour blocks and generated code can still be explored before installing a compiler.";
        }
    }
    ~App();
    void refresh_code() {
        if (importedSourceActive) {
            generated.code = importedSource;
            generated.errors.clear();
            codeLines = split_lines(importedSource);
            codeOwners.assign(codeLines.size(), -1);
        } else {
            generated = cpplab::generate_cpp(blocks);
            codeLines = split_lines(generated.code);
            codeOwners = generated.lineOwners;
        }
        codeScroll = std::min(codeScroll, std::max(0, static_cast<int>(codeLines.size()) - 1));
    }
    void remember() {
        undoStack.push_back({blocks, importedSource, importedSourceActive});
        if (undoStack.size() > 40) undoStack.erase(undoStack.begin());
    }
    void clamp_scrolls() {
        RECT rc{}; GetClientRect(hwnd, &rc);
        int w = rc.right, h = rc.bottom;
        int bodyTop = 78, bodyH = h - 78 - 36;
        int leftW = 224, centerW = std::max(370, static_cast<int>((w - 260) * 0.43));
        int centerX = 240;
        int rightX = centerX + centerW + 10;
        int rightW = w - rightX - 12;
        int paletteViewH = std::max(1, bodyH - 58 - 76);
        int paletteRows = (static_cast<int>(cpplab::block_catalog().size()) + PALETTE_COLUMNS - 1) / PALETTE_COLUMNS;
        int paletteContent = paletteRows * PALETTE_ROW_HEIGHT;
        paletteScroll = std::clamp(paletteScroll, 0, std::max(0, paletteContent - paletteViewH));
        int canvasViewH = bodyH - 72;
        int canvasContent = canvas_content_height(blocks);
        canvasScroll = std::clamp(canvasScroll, 0, std::max(0, canvasContent - canvasViewH));
        const int canvasViewW = std::max(1, centerW - 24);
        const int canvasContentW = std::max(GRID_CELL, canvas_max_grid_column(blocks) * GRID_CELL + 16);
        const int maxCanvasScrollX = std::max(0, canvasContentW - canvasViewW);
        const int gridAlignedMaxX = ((maxCanvasScrollX + GRID_CELL - 1) / GRID_CELL) * GRID_CELL;
        canvasScrollX = std::clamp(canvasScrollX, 0, gridAlignedMaxX);
        int split = bodyTop + bodyH * 57 / 100;
        int codeView = split - bodyTop - 73;
        int outView = bodyTop + bodyH - split - 69;
        codeScroll = std::clamp(codeScroll, 0, std::max(0, static_cast<int>(codeLines.size()) - std::max(1, codeView / 19)));
        auto outLines = wrap_output(output, std::max(24, (rightW - 41) / 7));
        outputScroll = std::clamp(outputScroll, 0, std::max(0, static_cast<int>(outLines.size()) - std::max(1, outView / 19)));
        (void)leftW; (void)rightW;
        int glossView = std::max(1, h - 178);
        glossaryScroll = std::clamp(glossaryScroll, 0, std::max(0, static_cast<int>(cpplab::block_catalog().size()) * GLOSSARY_ROW_HEIGHT - glossView));
    }
};

struct Hit {
    enum class Kind { Button, Palette, CanvasBlock, ScrollTrack, ScrollThumb, OverlayClose } kind;
    Rect rect;
    int value = 0;
    App::ScrollKind scrollKind = App::ScrollKind::None;
};

App::~App() {
    if (font) DeleteObject(font);
    if (smallFont) DeleteObject(smallFont);
    if (monoFont) DeleteObject(monoFont);
    if (titleFont) DeleteObject(titleFont);
    if (tileFont) DeleteObject(tileFont);
}

enum ActionId {
    A_RUN = 1, A_UNDO, A_EDIT, A_DELETE, A_UP, A_DOWN, A_INDENT, A_OUTDENT,
    A_EXAMPLE, A_GLOSSARY, A_HELP, A_SAVE, A_LOAD_CPP, A_CLEAR_ALL, A_CLOSE_OVERLAY
};

void add_hit(App& a, Hit::Kind kind, Rect r, int value = 0, App::ScrollKind sk = App::ScrollKind::None) {
    a.hits.push_back({kind, r, value, sk});
}
void fill_rect(HDC dc, Rect r, COLORREF c) {
    HBRUSH b = CreateSolidBrush(c); RECT nr = native_rect(r); FillRect(dc, &nr, b); DeleteObject(b);
}
void outline_rect(HDC dc, Rect r, COLORREF c, int width = 1) {
    HPEN p = CreatePen(PS_SOLID, width, c); HGDIOBJ oldP = SelectObject(dc, p); HGDIOBJ oldB = SelectObject(dc, GetStockObject(NULL_BRUSH));
    Rectangle(dc, r.x, r.y, r.x + r.w, r.y + r.h); SelectObject(dc, oldB); SelectObject(dc, oldP); DeleteObject(p);
}
void round_rect(HDC dc, Rect r, COLORREF fill, COLORREF border, int radius = 9, int borderWidth = 1) {
    HBRUSH b = CreateSolidBrush(fill); HPEN p = CreatePen(PS_SOLID, borderWidth, border);
    HGDIOBJ oldB = SelectObject(dc, b), oldP = SelectObject(dc, p);
    RoundRect(dc, r.x, r.y, r.x + r.w, r.y + r.h, radius, radius);
    SelectObject(dc, oldB); SelectObject(dc, oldP); DeleteObject(b); DeleteObject(p);
}
void draw_text(HDC dc, const std::string& text, Rect r, COLORREF color, HFONT font, UINT flags = DT_LEFT | DT_VCENTER | DT_SINGLELINE) {
    std::wstring wide = to_wide(text);
    SetBkMode(dc, TRANSPARENT); SetTextColor(dc, color);
    HGDIOBJ old = SelectObject(dc, font);
    RECT nr = native_rect(r);
    DrawTextW(dc, wide.c_str(), static_cast<int>(wide.size()), &nr, flags | DT_NOPREFIX);
    SelectObject(dc, old);
}
void draw_button(App& a, HDC dc, Rect r, const std::string& label, int action, bool primary = false, bool disabled = false) {
    COLORREF bg = disabled ? RGB(42, 47, 57) : (primary ? RGB(73, 112, 81) : RGB(40, 49, 64));
    COLORREF border = disabled ? RGB(53, 59, 71) : (primary ? C_GREEN : C_EDGE);
    round_rect(dc, r, bg, border, 8);
    draw_text(dc, label, {r.x + 3, r.y + 1, r.w - 6, r.h - 2}, disabled ? RGB(105, 115, 131) : C_TEXT, a.smallFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (!disabled) add_hit(a, Hit::Kind::Button, r, action);
}
void draw_panel(HDC dc, Rect r, const std::string& title, const std::string& subtitle, App& a) {
    round_rect(dc, r, C_PANEL, C_EDGE, 12);
    draw_text(dc, title, {r.x + 14, r.y + 9, r.w - 28, 23}, C_TEXT, a.font);
    draw_text(dc, subtitle, {r.x + 14, r.y + 31, r.w - 28, 18}, C_MUTED, a.smallFont);
}

struct ScrollLayout { Rect track; Rect thumb; int maxOffset = 0; int thumbTravel = 0; };
ScrollLayout draw_scrollbar(App& a, HDC dc, int x, int y, int h, int total, int view, int offset, App::ScrollKind kind) {
    ScrollLayout s;
    s.track = {x, y, 12, h};
    fill_rect(dc, s.track, RGB(24, 29, 39));
    s.maxOffset = std::max(0, total - view);
    int thumbH = total <= 0 ? h : std::max(28, static_cast<int>((static_cast<long long>(h) * view) / std::max(total, view)));
    thumbH = std::min(thumbH, h);
    s.thumbTravel = std::max(0, h - thumbH);
    int top = s.maxOffset == 0 ? 0 : static_cast<int>((static_cast<long long>(offset) * s.thumbTravel) / s.maxOffset);
    s.thumb = {x + 1, y + top, 10, thumbH};
    round_rect(dc, s.thumb, RGB(97, 112, 134), RGB(126, 141, 163), 6);
    // The hit area is wider than the visible scrollbar to make it easier to grab.
    add_hit(a, Hit::Kind::ScrollTrack, {x - 5, y, 22, h}, 0, kind);
    add_hit(a, Hit::Kind::ScrollThumb, {x - 5, s.thumb.y, 22, s.thumb.h}, 0, kind);
    return s;
}

void draw_header(App& a, HDC dc, int width) {
    fill_rect(dc, {0, 0, width, 70}, RGB(22, 27, 37));
    draw_text(dc, "C++ CIRCUIT LAB", {18, 8, 220, 28}, C_GOLD, a.titleFont);
    draw_text(dc, "Learn by building, breaking, fixing, and running real C++", {20, 36, 245, 18}, C_MUTED, a.smallFont);
    int x = 280;
    auto b = [&](const char* label, int w, int id, bool primary = false, bool disabled = false) {
        draw_button(a, dc, {x, 17, w, 35}, label, id, primary, disabled); x += w + 4;
    };
    b("Run F5", 64, A_RUN, true);
    b("Undo", 47, A_UNDO, false, a.undoStack.empty());
    b("Edit", 43, A_EDIT, false, a.selected < 0 || a.selected >= static_cast<int>(a.blocks.size()));
    b("Remove", 57, A_DELETE, false, a.selected < 0 || a.selected >= static_cast<int>(a.blocks.size()));
    b("↑", 29, A_UP, false, a.selected < 0);
    b("↓", 29, A_DOWN, false, a.selected < 0);
    b("←", 29, A_OUTDENT, false, a.selected < 0);
    b("→", 29, A_INDENT, false, a.selected < 0);
    b("Example", 63, A_EXAMPLE);
    b("Glossary", 68, A_GLOSSARY);
    b("Help", 44, A_HELP);
    b("Save C++", 72, A_SAVE);
    b("Load C++", 72, A_LOAD_CPP);
    b("Clear all", 68, A_CLEAR_ALL);
    (void)width;
}

// Shared renderer for palette and glossary previews. Both locations intentionally use
// identical dimensions, bevel, fill color, compact tile label, and text alignment so a
// learner can visually match a glossary entry to the exact tile in the main palette.
void draw_catalog_tile(App& a, HDC dc, Rect r, BlockType type, bool hover) {
    COLORREF fill = brighten(block_color(type), 22);
    if (hover) fill = brighten(fill, 18);
    draw_cube_tile(dc, r, fill, hover ? C_GOLD : RGB(20, 25, 34), hover ? 2 : 1);
    draw_text(dc, cpplab::tile_label(type), {r.x + 3, r.y + 8, r.w - 6, r.h - 14},
              C_TEXT, a.tileFont ? a.tileFont : a.smallFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void draw_palette(App& a, HDC dc, Rect panel, int bodyTop, int bodyBottom) {
    draw_panel(dc, panel, "ATOMIC BLOCKS", "One token or one simple action per tile", a);
    const auto& catalog = cpplab::block_catalog();
    int listTop = panel.y + 58;
    int listBottom = bodyBottom - 76;
    int listH = std::max(1, listBottom - listTop);
    Rect listView{panel.x + 6, listTop, panel.w - 24, listH};
    fill_rect(dc, listView, RGB(22, 28, 39));
    HRGN clip = CreateRectRgn(listView.x, listView.y, listView.x + listView.w, listView.y + listView.h);
    SelectClipRgn(dc, clip);

    const int paletteWidth = PALETTE_COLUMNS * TILE_SIZE + (PALETTE_COLUMNS - 1) * (PALETTE_COLUMN_PITCH - TILE_SIZE);
    const int startX = listView.x + std::max(2, (listView.w - paletteWidth) / 2);
    a.paletteRects.assign(catalog.size(), Rect{});
    for (size_t i = 0; i < catalog.size(); ++i) {
        const int col = static_cast<int>(i % PALETTE_COLUMNS);
        const int row = static_cast<int>(i / PALETTE_COLUMNS);
        Rect r{startX + col * PALETTE_COLUMN_PITCH,
               listTop + row * PALETTE_ROW_HEIGHT - a.paletteScroll,
               TILE_SIZE, TILE_SIZE};
        a.paletteRects[i] = r;
        if (r.y + r.h < listView.y || r.y > listView.y + listView.h) continue;
        const bool hover = contains(r, a.mouseX, a.mouseY);
        draw_catalog_tile(a, dc, r, catalog[i].type, hover);
        add_hit(a, Hit::Kind::Palette, r, static_cast<int>(catalog[i].type));
    }
    SelectClipRgn(dc, nullptr); DeleteObject(clip);
    const int paletteRows = (static_cast<int>(catalog.size()) + PALETTE_COLUMNS - 1) / PALETTE_COLUMNS;
    draw_scrollbar(a, dc, panel.x + panel.w - 15, listView.y,
        listView.h, paletteRows * PALETTE_ROW_HEIGHT, listView.h, a.paletteScroll, App::ScrollKind::Palette);
    Rect hint{panel.x + 11, bodyBottom - 70, panel.w - 22, 58};
    round_rect(dc, hint, RGB(31, 41, 51), RGB(55, 78, 85), 7);
    draw_text(dc, "Build code from separate pieces: if + ( + Name + == + Number + ) + {. Number, Text, Name, Type name, and Expression are reusable editable value tiles. Double-click a tile to edit its value.",
        {hint.x + 8, hint.y + 5, hint.w - 16, hint.h - 10}, C_MUTED, a.smallFont, DT_LEFT | DT_TOP | DT_WORDBREAK);
    (void)bodyTop;
}

void draw_canvas(App& a, HDC dc, Rect panel, int bodyTop, int bodyBottom) {
    std::string subtitle = a.handBlockType < 0 ? "Square grid · HAND: EMPTY · click a tile, then click a cell"
        : std::string("HAND: ") + cpplab::tile_label(static_cast<BlockType>(a.handBlockType)) +
          (a.handSticky ? " (kept) · click cells to place" : " · click one cell to place");
    draw_panel(dc, panel, "YOUR PROGRAM", subtitle, a);
    Rect view{panel.x + 8, panel.y + 58, panel.w - 24, panel.h - 72};
    HRGN clip = CreateRectRgn(view.x, view.y, view.x + view.w, view.y + view.h);
    SelectClipRgn(dc, clip);
    fill_rect(dc, view, RGB(22, 28, 39));

    // Draw fixed 48x48 grid cells. Tiles are square, with the same pitch on both axes.
    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(43, 52, 68));
    HGDIOBJ oldGridPen = SelectObject(dc, gridPen);
    const int gridOriginX = view.x + GRID_ORIGIN_INSET - a.canvasScrollX;
    const int gridOriginY = view.y + GRID_ORIGIN_INSET - a.canvasScroll;
    const auto positiveMod = [](int value, int divisor) { int r = value % divisor; return r < 0 ? r + divisor : r; };
    const int firstX = view.x + positiveMod(gridOriginX - view.x, GRID_CELL);
    const int firstY = view.y + positiveMod(gridOriginY - view.y, GRID_CELL);
    for (int gx = firstX; gx < view.x + view.w; gx += GRID_CELL) {
        MoveToEx(dc, gx, view.y, nullptr); LineTo(dc, gx, view.y + view.h);
    }
    for (int gy = firstY; gy < view.y + view.h; gy += GRID_CELL) {
        MoveToEx(dc, view.x, gy, nullptr); LineTo(dc, view.x + view.w, gy);
    }
    SelectObject(dc, oldGridPen); DeleteObject(gridPen);

    a.blockRects.assign(a.blocks.size(), Rect{});
    a.hoveredCanvas = -1;
    const bool atomicMode = atomic_canvas_mode(a.blocks);
    for (size_t i = 0; i < a.blocks.size(); ++i) {
        const Block& b = a.blocks[i];
        const int depth = std::clamp(b.depth, 0, 6);
        const int tileX = atomicMode ? std::max(0, b.gridX) : depth;
        const int tileY = atomicMode ? std::max(0, b.gridY) : static_cast<int>(i);
        Rect r{view.x + BLOCK_ORIGIN_INSET + tileX * GRID_CELL - a.canvasScrollX,
               view.y + BLOCK_ORIGIN_INSET + tileY * GRID_CELL - a.canvasScroll,
               TILE_SIZE, TILE_SIZE};
        a.blockRects[i] = r;
        if (r.y + r.h < view.y || r.y > view.y + view.h || r.x + r.w < view.x || r.x > view.x + view.w) continue;
        const bool selected = static_cast<int>(i) == a.selected;
        const bool draggingThis = a.dragSource == static_cast<int>(i) &&
            (std::abs(a.mouseY - a.dragStartY) > 5 || std::abs(a.mouseX - a.dragStartX) > 5);
        const bool hover = contains(r, a.mouseX, a.mouseY);
        if (hover && !draggingThis) a.hoveredCanvas = static_cast<int>(i);
        if (!draggingThis) {
            COLORREF fill = brighten(block_color(b.type), 20);
            if (hover) fill = brighten(fill, 14);
            draw_cube_tile(dc, r, fill, selected ? C_GOLD : (hover ? RGB(224, 232, 242) : RGB(18, 23, 32)), selected ? 2 : 1);
            draw_text(dc, tile_face_label(b), {r.x + 2, r.y + 7, r.w - 4, r.h - 12},
                      C_TEXT, a.tileFont ? a.tileFont : a.smallFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        if (!draggingThis) {
            const int hx = std::max(r.x, view.x);
            const int hy = std::max(r.y, view.y);
            const int hr = std::min(r.x + r.w, view.x + view.w);
            const int hb = std::min(r.y + r.h, view.y + view.h);
            if (hr > hx && hb > hy) add_hit(a, Hit::Kind::CanvasBlock, {hx, hy, hr - hx, hb - hy}, static_cast<int>(i));
        }
    }

    const bool movingCanvasTile = a.dragSource >= 0 && a.dragSource < static_cast<int>(a.blocks.size()) &&
        (std::abs(a.mouseY - a.dragStartY) > 5 || std::abs(a.mouseX - a.dragStartX) > 5);
    const bool movingPaletteTile = a.paletteDragType >= 0 && contains(view, a.mouseX, a.mouseY) &&
        (std::abs(a.mouseY - a.paletteDragStartY) > 5 || std::abs(a.mouseX - a.paletteDragStartX) > 5);
    const bool hoveringHandCell = a.handBlockType >= 0 && contains(view, a.mouseX, a.mouseY) &&
        !movingCanvasTile && !movingPaletteTile;
    if (movingCanvasTile || movingPaletteTile || hoveringHandCell) {
        const bool fromCanvas = movingCanvasTile;
        const bool fromPalette = !fromCanvas && movingPaletteTile;
        const BlockType type = fromCanvas
            ? a.blocks[static_cast<size_t>(a.dragSource)].type
            : (fromPalette ? static_cast<BlockType>(a.paletteDragType) : static_cast<BlockType>(a.handBlockType));
        int snapDepth = 0;
        int snapRow = 0;
        if (fromCanvas || fromPalette) {
            const int grabX = fromCanvas ? a.dragGrabX : a.paletteDragGrabX;
            const int grabY = fromCanvas ? a.dragGrabY : a.paletteDragGrabY;
            snapDepth = std::clamp(static_cast<int>(std::lround(
                (a.mouseX - grabX - (view.x + BLOCK_ORIGIN_INSET) + a.canvasScrollX) / static_cast<double>(GRID_CELL))), 0,
                std::max(canvas_max_grid_column(a.blocks), (a.canvasScrollX + view.w) / GRID_CELL));
            snapRow = std::clamp(static_cast<int>(std::lround(
                (a.mouseY - grabY - (view.y + BLOCK_ORIGIN_INSET) + a.canvasScroll) / static_cast<double>(GRID_CELL))), 0,
                std::max(canvas_max_grid_row(a.blocks), (a.canvasScroll + view.h) / GRID_CELL));
        } else {
            snapDepth = std::clamp((a.mouseX - (view.x + GRID_ORIGIN_INSET) + a.canvasScrollX) / GRID_CELL, 0,
                                   std::max(canvas_max_grid_column(a.blocks), (a.canvasScrollX + view.w) / GRID_CELL));
            snapRow = std::clamp((a.mouseY - (view.y + GRID_ORIGIN_INSET) + a.canvasScroll) / GRID_CELL,
                                 0, std::max(canvas_max_grid_row(a.blocks), (a.canvasScroll + view.h) / GRID_CELL));
        }
        Rect ghost{view.x + BLOCK_ORIGIN_INSET + snapDepth * GRID_CELL - a.canvasScrollX,
                   view.y + BLOCK_ORIGIN_INSET + snapRow * CANVAS_PITCH - a.canvasScroll,
                   TILE_SIZE, TILE_SIZE};
        draw_cube_tile(dc, ghost, brighten(block_color(type), 28), C_GOLD, 2);
        draw_text(dc, cpplab::tile_label(type), {ghost.x + 3, ghost.y + 8, ghost.w - 6, ghost.h - 14},
                  C_TEXT, a.tileFont ? a.tileFont : a.smallFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    SelectClipRgn(dc, nullptr); DeleteObject(clip);
    const int content = canvas_content_height(a.blocks);
    draw_scrollbar(a, dc, panel.x + panel.w - 16, view.y, view.h, content, view.h, a.canvasScroll, App::ScrollKind::Canvas);
    if (a.selected >= 0 && a.selected < static_cast<int>(a.blocks.size())) {
        const auto& b = a.blocks[static_cast<size_t>(a.selected)];
        const int visibleColumn = atomic_canvas_mode(a.blocks) ? b.gridX : b.depth;
        const std::string summary = std::string("Selected: ") + cpplab::block_name(b.type) + " — " + block_summary(b) +
                                    "  |  column " + std::to_string(visibleColumn + 1);
        draw_text(dc, shorten(summary, static_cast<size_t>(std::max(24, (panel.w - 30) / 7))),
                  {panel.x + 13, panel.y + panel.h - 18, panel.w - 30, 15}, C_GOLD, a.smallFont);
    } else {
        draw_text(dc, "No block selected — your hand is empty.",
                  {panel.x + 13, panel.y + panel.h - 18, panel.w - 30, 15}, C_MUTED, a.smallFont);
    }
    (void)bodyTop; (void)bodyBottom;
}

void draw_code_panel(App& a, HDC dc, Rect panel) {
    draw_panel(dc, panel, "REAL C++ CODE", "These are the lines your blocks create", a);
    Rect view{panel.x + 10, panel.y + 57, panel.w - 25, panel.h - 66};
    HRGN clip = CreateRectRgn(view.x, view.y, view.x + view.w, view.y + view.h);
    SelectClipRgn(dc, clip);
    int visible = std::max(1, view.h / 19);
    for (int row = 0; row < visible; ++row) {
        int idx = a.codeScroll + row;
        if (idx >= static_cast<int>(a.codeLines.size())) break;
        int owner = idx < static_cast<int>(a.codeOwners.size()) ? a.codeOwners[idx] : -1;
        bool highlight = owner >= 0 && owner == a.lastRunBlock;
        if (highlight) fill_rect(dc, {view.x, view.y + row * 19, view.w - 1, 19}, RGB(60, 77, 63));
        draw_text(dc, std::to_string(idx + 1), {view.x + 1, view.y + row * 19, 31, 18}, RGB(106, 120, 140), a.smallFont);
        draw_text(dc, a.codeLines[idx], {view.x + 35, view.y + row * 19, view.w - 40, 18},
            highlight ? RGB(238, 245, 223) : RGB(196, 211, 228), a.monoFont);
    }
    SelectClipRgn(dc, nullptr); DeleteObject(clip);
    draw_scrollbar(a, dc, panel.x + panel.w - 15, view.y, view.h,
        static_cast<int>(a.codeLines.size()), std::max(1, view.h / 19), a.codeScroll, App::ScrollKind::Code);
}

std::vector<std::string> wrap_output(const std::string& text, int chars) {
    std::vector<std::string> out;
    std::istringstream input(text);
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) { out.emplace_back(); continue; }
        while (line.size() > static_cast<size_t>(chars)) {
            size_t cut = line.rfind(' ', static_cast<size_t>(chars));
            if (cut == std::string::npos || cut < static_cast<size_t>(chars / 3)) cut = static_cast<size_t>(chars);
            out.push_back(line.substr(0, cut));
            line.erase(0, cut);
            while (!line.empty() && line.front() == ' ') line.erase(line.begin());
        }
        out.push_back(line);
    }
    if (out.empty()) out.emplace_back();
    return out;
}
void draw_output_panel(App& a, HDC dc, Rect panel) {
    draw_panel(dc, panel, "TRY IT: OUTPUT & FEEDBACK", a.compilerStatus, a);
    Rect view{panel.x + 11, panel.y + 57, panel.w - 26, panel.h - 67};
    HRGN clip = CreateRectRgn(view.x, view.y, view.x + view.w, view.y + view.h);
    SelectClipRgn(dc, clip);
    int charCount = std::max(24, (view.w - 15) / 7);
    auto lines = wrap_output(a.output, charCount);
    int visible = std::max(1, view.h / 19);
    for (int row = 0; row < visible; ++row) {
        int idx = a.outputScroll + row;
        if (idx >= static_cast<int>(lines.size())) break;
        COLORREF color = (lines[idx].find("[ERROR]") != std::string::npos || lines[idx].find("error:") != std::string::npos) ? C_RED : C_TEXT;
        draw_text(dc, lines[idx], {view.x + 2, view.y + row * 19, view.w - 10, 18}, color, a.monoFont);
    }
    SelectClipRgn(dc, nullptr); DeleteObject(clip);
    draw_scrollbar(a, dc, panel.x + panel.w - 15, view.y, view.h,
        static_cast<int>(lines.size()), std::max(1, view.h / 19), a.outputScroll, App::ScrollKind::Output);
}


Rect canvas_view_for(HWND hwnd);

void draw_hover_tooltip(App& a, HDC dc, int w, int h) {
    if (a.overlayGlossary || a.overlayHelp) return;
    const Hit* target = nullptr;
    if (a.handBlockType >= 0 && contains(canvas_view_for(a.hwnd), a.mouseX, a.mouseY)) {
        const BlockInfo* info = cpplab::block_info(static_cast<BlockType>(a.handBlockType));
        if (info) {
            const int boxW = std::min(360, std::max(240, w - 24));
            const int boxH = 58;
            int x = a.mouseX + 18, y = a.mouseY + 18;
            if (x + boxW > w - 8) x = a.mouseX - boxW - 18;
            if (y + boxH > h - 35) y = a.mouseY - boxH - 12;
            x = std::clamp(x, 8, std::max(8, w - boxW - 8));
            y = std::clamp(y, 8, std::max(8, h - boxH - 35));
            round_rect(dc, {x, y, boxW, boxH}, RGB(35, 43, 57), C_GOLD, 7, 1);
            draw_text(dc, info->name, {x + 10, y + 5, boxW - 20, 19}, C_GOLD, a.font);
            draw_text(dc, info->shortDescription, {x + 10, y + 27, boxW - 20, 23}, C_TEXT, a.smallFont, DT_LEFT | DT_TOP | DT_WORDBREAK);
            return;
        }
    }
    for (auto it = a.hits.rbegin(); it != a.hits.rend(); ++it) {
        if (!contains(it->rect, a.mouseX, a.mouseY)) continue;
        if (it->kind == Hit::Kind::Palette || it->kind == Hit::Kind::CanvasBlock) {
            target = &*it;
            break;
        }
    }
    if (!target) return;
    BlockType type{};
    if (target->kind == Hit::Kind::Palette) {
        type = static_cast<BlockType>(target->value);
    } else {
        if (target->value < 0 || target->value >= static_cast<int>(a.blocks.size())) return;
        type = a.blocks[static_cast<size_t>(target->value)].type;
    }
    const BlockInfo* info = cpplab::block_info(type);
    if (!info) return;
    const int boxW = std::min(360, std::max(240, w - 24));
    const int boxH = 58;
    int x = a.mouseX + 18;
    int y = a.mouseY + 18;
    if (x + boxW > w - 8) x = a.mouseX - boxW - 18;
    if (y + boxH > h - 35) y = a.mouseY - boxH - 12;
    x = std::clamp(x, 8, std::max(8, w - boxW - 8));
    y = std::clamp(y, 8, std::max(8, h - boxH - 35));
    round_rect(dc, {x, y, boxW, boxH}, RGB(35, 43, 57), C_GOLD, 7, 1);
    draw_text(dc, info->name, {x + 10, y + 5, boxW - 20, 19}, C_GOLD, a.font);
    draw_text(dc, info->shortDescription, {x + 10, y + 27, boxW - 20, 23}, C_TEXT, a.smallFont, DT_LEFT | DT_TOP | DT_WORDBREAK);
}

void draw_glossary(App& a, HDC dc, int w, int h) {
    fill_rect(dc, {0, 0, w, h}, RGB(12, 15, 21));
    Rect panel{48, 40, w - 96, h - 80};
    round_rect(dc, panel, C_PANEL, C_GOLD, 14, 2);
    draw_text(dc, "BLOCK GLOSSARY — plain language, real C++", {panel.x + 20, panel.y + 14, panel.w - 120, 28}, C_GOLD, a.titleFont);
    draw_text(dc, "Scroll with the wheel, click the bar, drag its thumb, or press Esc to close.",
        {panel.x + 21, panel.y + 44, panel.w - 140, 21}, C_MUTED, a.smallFont);
    draw_button(a, dc, {panel.x + panel.w - 92, panel.y + 13, 72, 31}, "Close  Esc", A_CLOSE_OVERLAY);
    Rect view{panel.x + 18, panel.y + 78, panel.w - 48, panel.h - 98};
    HRGN clip = CreateRectRgn(view.x, view.y, view.x + view.w, view.y + view.h);
    SelectClipRgn(dc, clip);
    const auto& entries = cpplab::block_catalog();
    const int rowHeight = GLOSSARY_ROW_HEIGHT;
    for (size_t i = 0; i < entries.size(); ++i) {
        Rect r{view.x, view.y + static_cast<int>(i) * rowHeight - a.glossaryScroll, view.w - 8, rowHeight - 10};
        if (r.y + r.h < view.y || r.y > view.y + view.h || r.x + r.w < view.x || r.x > view.x + view.w) continue;
        round_rect(dc, r, RGB(32, 39, 52), C_EDGE, 8);
        // Show the exact same square tile the learner sees in the left-hand library.
        // There is deliberately no hover behavior or tooltip in the glossary preview.
        Rect tile{r.x + 15, r.y + 16, TILE_SIZE, TILE_SIZE};
        draw_catalog_tile(a, dc, tile, entries[i].type, false);

        // Keep detailed text in a separate column so the tile stays visually distinct.
        // Entries and palette tiles both come from block_catalog(), avoiding duplicate lists.
        const int textX = r.x + 76;
        const int textW = r.w - 88;
        draw_text(dc, entries[i].name, {textX, r.y + 7, textW, 21}, C_TEXT, a.font);
        draw_text(dc, std::string("WHAT IT DOES: ") + entries[i].longDescription,
            {textX, r.y + 31, textW, 44}, C_MUTED, a.smallFont, DT_LEFT | DT_TOP | DT_WORDBREAK);
        draw_text(dc, std::string("EXAMPLE: ") + entries[i].example,
            {textX, r.y + 78, textW, 39}, RGB(171, 211, 180), a.monoFont, DT_LEFT | DT_TOP | DT_WORDBREAK);
        draw_text(dc, std::string("WATCH OUT: ") + entries[i].warning,
            {textX, r.y + 120, textW, 43}, RGB(232, 189, 131), a.smallFont, DT_LEFT | DT_TOP | DT_WORDBREAK);
    }
    SelectClipRgn(dc, nullptr); DeleteObject(clip);
    draw_scrollbar(a, dc, panel.x + panel.w - 23, view.y, view.h,
        static_cast<int>(entries.size()) * rowHeight, view.h, a.glossaryScroll, App::ScrollKind::Glossary);
}

void draw_help(App& a, HDC dc, int w, int h) {
    fill_rect(dc, {0, 0, w, h}, RGB(12, 15, 21));
    Rect panel{90, 60, w - 180, h - 120};
    round_rect(dc, panel, C_PANEL, C_GOLD, 14, 2);
    draw_text(dc, "HOW TO LEARN C++ HERE", {panel.x + 22, panel.y + 17, panel.w - 120, 30}, C_GOLD, a.titleFont);
    draw_button(a, dc, {panel.x + panel.w - 93, panel.y + 15, 72, 32}, "Close  Esc", A_CLOSE_OVERLAY);
    std::vector<std::pair<std::string, std::string>> rows = {
        {"1. Pick up a tile", "Click a library tile to put it in your hand, then click a grid cell to place it. Shift-click keeps it in your hand for multiple placements; Esc or right-click empties your hand."},
        {"2. Change it", "Select a block and click Edit, or double-click it. Try changing one number or word."},
        {"3. Build lines from tiny pieces", "Every syntax tile is one word or symbol. Name, Type name, Number, Text, Character, and Expression are reusable value slots: each copy can be edited to hold a different value. Place tiles left-to-right in one grid row to build a source line."},
        {"4. Predict, then Run", "Say what you think will happen before pressing Run (F5). Compare your prediction with the output."},
        {"5. Make a mistake on purpose", "Try a bad expression like 2 + ;, then run it. Read the first compiler error, edit the block, and run again."},
        {"6. Read the C++", "The right panel shows the exact source built from the grid. A Number tile is one editable number, not a list of preset numbers. Text, Name, Type name, and Expression work the same way. Build statements left-to-right and use separate punctuation tiles."},
        {"C++ facts", "; ends most statements. == compares. = stores a value. Put separate { and } tiles into the grid to open and close code groups. The generator indents source from those braces; grid columns only order the pieces within a source line."},
        {"Safety", "Run executes the C++ you create on your own computer. Only run code you trust. Endless loops are stopped after a short time."},
        {"Keyboard", "F5 Run · Enter Edit · Delete Remove · Ctrl+Z Undo · Ctrl+S Save .cpp · G Glossary · F1 Help · Esc Close"}
    };
    int y = panel.y + 66;
    for (const auto& row : rows) {
        draw_text(dc, row.first, {panel.x + 25, y, panel.w - 50, 22}, C_TEXT, a.font);
        draw_text(dc, row.second, {panel.x + 25, y + 23, panel.w - 50, 32}, C_MUTED, a.smallFont, DT_LEFT | DT_TOP | DT_WORDBREAK);
        y += 64;
    }
}

void paint(App& a) {
    RECT rc{}; GetClientRect(a.hwnd, &rc);
    int w = rc.right, h = rc.bottom;
    HDC target = GetDC(a.hwnd);
    HDC buffer = CreateCompatibleDC(target);
    HBITMAP bitmap = CreateCompatibleBitmap(target, std::max(1, w), std::max(1, h));
    HGDIOBJ oldBitmap = SelectObject(buffer, bitmap);
    fill_rect(buffer, {0, 0, w, h}, C_BG);
    a.hits.clear();
    a.blockRects.clear();
    draw_header(a, buffer, w);
    const int footerH = 31;
    int bodyTop = 78, bodyBottom = h - footerH - 5, bodyH = bodyBottom - bodyTop;
    int leftX = 12, leftW = 218, centerX = leftX + leftW + 10;
    int centerW = std::max(370, static_cast<int>((w - 260) * 0.43));
    int rightX = centerX + centerW + 10;
    int rightW = std::max(280, w - rightX - 12);
    Rect left{leftX, bodyTop, leftW, bodyH};
    Rect center{centerX, bodyTop, centerW, bodyH};
    int split = bodyTop + bodyH * 57 / 100;
    Rect code{rightX, bodyTop, rightW, split - bodyTop - 7};
    Rect output{rightX, split + 2, rightW, bodyBottom - split - 2};
    draw_palette(a, buffer, left, bodyTop, bodyBottom);
    draw_canvas(a, buffer, center, bodyTop, bodyBottom);
    draw_code_panel(a, buffer, code);
    draw_output_panel(a, buffer, output);
    draw_hover_tooltip(a, buffer, w, h);
    fill_rect(buffer, {0, h - footerH, w, footerH}, RGB(22, 27, 37));
    draw_text(buffer, shorten(a.status, static_cast<size_t>(std::max(70, w / 7))), {14, h - footerH + 3, w - 28, 23}, C_MUTED, a.smallFont);
    if (a.overlayGlossary) draw_glossary(a, buffer, w, h);
    if (a.overlayHelp) draw_help(a, buffer, w, h);
    BitBlt(target, 0, 0, w, h, buffer, 0, 0, SRCCOPY);
    SelectObject(buffer, oldBitmap); DeleteObject(bitmap); DeleteDC(buffer); ReleaseDC(a.hwnd, target);
}

void refresh_and_draw(App& a, bool keepImportedSource = false) {
    if (!keepImportedSource) { a.importedSourceActive = false; a.importedSource.clear(); }
    a.refresh_code(); a.clamp_scrolls(); InvalidateRect(a.hwnd, nullptr, FALSE);
}
void set_output(App& a, const std::string& text) { a.output = text; a.outputScroll = 0; a.clamp_scrolls(); InvalidateRect(a.hwnd, nullptr, FALSE); }
bool add_block(App& a, BlockType type) {
    const bool hasFullFile = std::any_of(a.blocks.begin(), a.blocks.end(), [](const Block& b) { return b.type == BlockType::RawProgram; });
    if (hasFullFile && type != BlockType::RawProgram) {
        MessageBoxW(a.hwnd, L"A C++ file block supplies the entire program. Remove it first to go back to visual blocks.",
            L"Whole-file mode is active", MB_OK | MB_ICONINFORMATION);
        return false;
    }
    if (type == BlockType::RawProgram && !a.blocks.empty()) {
        int answer = MessageBoxW(a.hwnd,
            L"A C++ file block replaces the current visual program with a complete source file. Continue? Undo can restore the current blocks.",
            L"Switch to complete C++ file mode?", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
        if (answer != IDYES) return false;
    }
    a.remember();
    Block block = cpplab::make_block(type);
    size_t insertAt = a.blocks.size();
    if (type == BlockType::RawProgram) {
        a.blocks.clear();
        a.selected = -1;
        insertAt = 0;
    } else if (a.selected >= 0 && a.selected < static_cast<int>(a.blocks.size())) {
        insertAt = static_cast<size_t>(a.selected + 1);
        const Block& current = a.blocks[a.selected];
        if (type == BlockType::Else || type == BlockType::ElseIf) {
            if (current.type == BlockType::If || current.type == BlockType::ElseIf) {
                // Branch blocks belong after the whole body of the preceding branch.
                block.depth = current.depth;
                insertAt = static_cast<size_t>(a.selected + 1);
                while (insertAt < a.blocks.size() && a.blocks[insertAt].depth > current.depth) ++insertAt;
            } else {
                block.depth = std::max(0, current.depth - 1);
            }
        } else if (cpplab::opens_scope(current.type)) {
            block.depth = current.depth + 1;
        } else {
            block.depth = current.depth;
        }
    } else if (!a.blocks.empty() && cpplab::opens_scope(a.blocks.back().type)) {
        block.depth = a.blocks.back().depth + 1;
    }
    if (type == BlockType::IncludeHeader) block.depth = 0;
    if (type == BlockType::RawProgram) block.depth = 0;
    a.blocks.insert(a.blocks.begin() + static_cast<std::ptrdiff_t>(insertAt), block);
    a.selected = static_cast<int>(insertAt);
    a.canvasScroll = std::max(0, a.selected * CANVAS_PITCH - 200);
    a.status = "Added square tile: " + std::string(cpplab::block_name(type)) + ". Hover for a short hint; open Glossary for details.";
    refresh_and_draw(a);
    return true;
}


Rect canvas_view_for(HWND hwnd) {
    RECT client{}; GetClientRect(hwnd, &client);
    const int bodyTop = 78;
    const int bodyBottom = client.bottom - 31 - 5;
    const int bodyH = bodyBottom - bodyTop;
    const int centerX = 240;
    const int centerW = std::max(370, static_cast<int>((client.right - 260) * 0.43));
    return {centerX + 8, bodyTop + 58, centerW - 24, bodyH - 72};
}

int grid_row_from_point(const Rect& view, int y, int scroll, int maxRow) {
    const int logicalY = y - (view.y + GRID_ORIGIN_INSET) + scroll;
    const int row = logicalY < 0 ? 0 : logicalY / GRID_CELL;
    return std::clamp(row, 0, maxRow);
}

int grid_column_from_point(const Rect& view, int x, int scrollX = 0, int maxColumn = 0) {
    const int logicalX = x - (view.x + GRID_ORIGIN_INSET) + scrollX;
    const int col = logicalX < 0 ? 0 : logicalX / GRID_CELL;
    return std::clamp(col, 0, std::max(maxColumn, (scrollX + view.w) / GRID_CELL));
}

// Add through the existing undo/safety path, then place the tile at an exact grid cell.
bool place_block_at_grid(App& a, BlockType type, int row, int depth) {
    if (!add_block(a, type)) return false;
    if (a.selected < 0 || a.selected >= static_cast<int>(a.blocks.size()) ||
        a.blocks[static_cast<size_t>(a.selected)].type != type) return false;

    const int source = a.selected;
    Block block = a.blocks[static_cast<size_t>(source)];
    a.blocks.erase(a.blocks.begin() + source);
    const bool atomicTile = cpplab::is_atomic_mode_type(type);
    const int requestedRow = std::max(0, row);

    if (atomicTile) {
        // The friendly action blocks (Set number, Print text, etc.) are one complete
        // learning action each, so they own a whole row and always begin in column zero.
        const bool action = cpplab::is_atomic_action_type(type);
        const int column = action ? 0 : std::clamp(depth, 0, 1000);
        const bool rowHasAction = std::any_of(a.blocks.begin(), a.blocks.end(), [&](const Block& b) {
            return b.gridY == requestedRow && cpplab::is_atomic_action_type(b.type);
        });
        const bool rowHasAnything = std::any_of(a.blocks.begin(), a.blocks.end(), [&](const Block& b) {
            return b.gridY == requestedRow;
        });
        const bool needsOwnRow = action ? rowHasAnything : rowHasAction;

        if (needsOwnRow) {
            // Insert a whole source line instead of silently mixing an action with tokens.
            for (auto& existing : a.blocks)
                if (existing.gridY >= requestedRow) ++existing.gridY;
        } else {
            const bool occupied = std::any_of(a.blocks.begin(), a.blocks.end(), [&](const Block& b) {
                return b.gridY == requestedRow && b.gridX == column;
            });
            if (occupied) {
                std::vector<size_t> shift;
                for (size_t k = 0; k < a.blocks.size(); ++k)
                    if (a.blocks[k].gridY == requestedRow && a.blocks[k].gridX >= column) shift.push_back(k);
                std::sort(shift.begin(), shift.end(), [&](size_t left, size_t right) {
                    return a.blocks[left].gridX > a.blocks[right].gridX;
                });
                for (size_t k : shift) ++a.blocks[k].gridX;
            }
        }
        block.gridX = column;
        block.gridY = requestedRow;
        block.depth = 0;
        a.blocks.push_back(std::move(block));
        a.selected = static_cast<int>(a.blocks.size()) - 1;
    } else {
        int insertAt = std::clamp(row, 0, static_cast<int>(a.blocks.size()));
        if (type == BlockType::IncludeHeader || type == BlockType::RawProgram) insertAt = 0;
        block.depth = (type == BlockType::IncludeHeader || type == BlockType::RawProgram) ? 0 : std::clamp(depth, 0, 6);
        a.blocks.insert(a.blocks.begin() + insertAt, std::move(block));
        a.selected = insertAt;
    }

    const Block& placed = a.blocks[static_cast<size_t>(a.selected)];
    const int actualColumn = atomicTile ? placed.gridX : placed.depth;
    a.status = "Placed " + std::string(cpplab::block_name(type)) + " on grid row " +
               std::to_string(atomicTile ? requestedRow + 1 : a.selected + 1) + ", column " +
               std::to_string(actualColumn + 1) + ".";
    if (!a.handSticky || type == BlockType::RawProgram) {
        a.handBlockType = -1;
        a.handSticky = false;
    }
    if (a.handBlockType >= 0) a.status += " Hand kept: " + std::string(cpplab::tile_label(static_cast<BlockType>(a.handBlockType))) + ".";
    else a.status += " Hand: empty.";
    a.canvasScroll = std::max(0, (atomicTile ? requestedRow : a.selected) * GRID_CELL - 200);
    refresh_and_draw(a);
    return true;
}

bool edit_block(App& a, int index) {
    if (index < 0 || index >= static_cast<int>(a.blocks.size())) return false;
    Block edited = a.blocks[index];
    auto ask = [&](const wchar_t* title, const wchar_t* label, std::string& field, bool multiline = false) {
        std::string tmp;
        if (!prompt_input(a.hwnd, title, label, field, tmp, multiline)) return false;
        field = std::move(tmp);
        return true;
    };
    bool ok = true;
    switch (edited.type) {
        case BlockType::IncludeHeader:
            ok = ask(L"Include header", L"Header name (example: vector, cmath, or \"helpers.hpp\"):", edited.value); break;
        case BlockType::PrintText:
            ok = ask(L"Print text", L"What words should the program print?", edited.value); break;
        case BlockType::PrintExpression:
            ok = ask(L"Print result", L"Type a C++ expression (example: 2 + 3 * 4).", edited.value); break;
        case BlockType::DeclareInt:
            ok = ask(L"Whole number", L"Name this whole-number variable (example: score).", edited.value) &&
                 ask(L"Starting value", L"What whole-number expression should it start with?", edited.extra); break;
        case BlockType::DeclareDouble:
            ok = ask(L"Decimal number", L"Name this decimal-number variable (example: distance).", edited.value) &&
                 ask(L"Starting value", L"What numeric expression should it start with?", edited.extra); break;
        case BlockType::DeclareText:
            ok = ask(L"Text", L"Name this text variable (example: player).", edited.value) &&
                 ask(L"Starting text", L"What words should be stored in it?", edited.extra); break;
        case BlockType::DeclareBool:
            ok = ask(L"True/false", L"Name this true/false variable.", edited.value) &&
                 ask(L"Starting value", L"Enter true or false (without quotes).", edited.extra); break;
        case BlockType::DeclareChar:
            ok = ask(L"Character", L"Name this character variable.", edited.value) &&
                 ask(L"Starting character", L"Enter one character, such as A or ?. You do not need quotes.", edited.extra); break;
        case BlockType::DeclareIntList:
            ok = ask(L"Number list", L"Name this list of whole numbers.", edited.value) &&
                 ask(L"Starting items", L"Enter comma-separated integers, such as 10, 20, 30.", edited.extra); break;
        case BlockType::ShowVariable:
            ok = ask(L"Show value", L"Type a variable name or expression to print.", edited.value); break;
        case BlockType::SetVariable:
            ok = ask(L"Set value", L"Which existing variable should change?", edited.value) &&
                 ask(L"New value", L"Type the expression to store (example: score + 1).", edited.extra); break;
        case BlockType::ReadInput:
            ok = ask(L"Read input", L"Which existing variable should receive the typed input?", edited.value); break;
        case BlockType::If:
            ok = ask(L"If condition", L"Type a test, such as score < 5 or score == 0.", edited.value); break;
        case BlockType::ElseIf:
            ok = ask(L"Else if condition", L"Test this only if earlier tests were false.", edited.value); break;
        case BlockType::Else:
            a.status = "Else has no value. It belongs at the same grid column as its matching If.";
            InvalidateRect(a.hwnd, nullptr, FALSE);
            return false;
        case BlockType::Repeat:
            ok = ask(L"Repeat count", L"How many times? Enter a number or numeric expression.", edited.value); break;
        case BlockType::ForLoop:
            ok = ask(L"For loop header", L"Enter init; condition; update (without the word for). Example: int i = 0; i < 5; ++i", edited.value); break;
        case BlockType::While:
            ok = ask(L"While condition", L"Type a test that can become false, such as score < 10.", edited.value); break;
        case BlockType::Break:
            a.status = "Break exits the nearest loop. Place it inside Repeat, For loop, or While.";
            InvalidateRect(a.hwnd, nullptr, FALSE);
            return false;
        case BlockType::Continue:
            a.status = "Continue skips to the next pass. Place it inside Repeat, For loop, or While.";
            InvalidateRect(a.hwnd, nullptr, FALSE);
            return false;
        case BlockType::Return:
            ok = ask(L"Return value", L"What value should this function return? Use 0 to finish main successfully.", edited.value); break;
        case BlockType::RawStatement:
            ok = ask(L"C++ line", L"Write one or more lines of C++ that run inside main. Real C++ rules apply.", edited.value, true); break;
        case BlockType::RawProgram:
            ok = ask(L"C++ file", L"Write a complete C++ source file. This replaces the visual program and must include any headers and main function you need.", edited.value, true); break;
        case BlockType::Comment:
            ok = ask(L"Comment", L"Write a note for people. The program will not print it.", edited.value); break;
        case BlockType::TokIdentifier:
            ok = ask(L"Name", L"Enter one C++ name, such as score, player, or calculate. Each Name tile can have a different value.", edited.value); break;
        case BlockType::TokTypeName:
            ok = ask(L"Type name", L"Enter a type such as int, double, std::string, std::vector<int>, or a type you created.", edited.value); break;
        case BlockType::TokExpression:
            ok = ask(L"Expression", L"Enter a value or calculation, such as score + 1. Do not enter a whole statement or its final semicolon.", edited.value); break;
        case BlockType::TokKeyword:
            ok = ask(L"C++ keyword", L"Enter one C++ keyword, such as constexpr, unsigned, sizeof, requires, or co_await.", edited.value); break;
        case BlockType::TokNumber:
            ok = ask(L"Number", L"Enter any number, such as 0, 42, -3, 3.14, or 0xFF. You do not need a unique block for each number.", edited.value); break;
        case BlockType::TokStringLiteral:
            ok = ask(L"Text", L"Enter any words. The editor adds double quotes and escapes special characters.", edited.value); break;
        case BlockType::TokCharLiteral:
            ok = ask(L"Character", L"Enter one character, such as A or ?. The editor adds single quotes.", edited.value); break;
        case BlockType::ActionSetNumber:
            ok = ask(L"Set number", L"Variable name, such as score.", edited.value) &&
                 ask(L"Starting number", L"Initial number, such as 0 or 10.", edited.extra); break;
        case BlockType::ActionSetText:
            ok = ask(L"Set text", L"Variable name, such as message.", edited.value) &&
                 ask(L"Starting text", L"The words to store (no quotes needed).", edited.extra); break;
        case BlockType::ActionPrintText:
            ok = ask(L"Print text", L"What words should appear in the output?", edited.value); break;
        case BlockType::ActionShowValue:
            ok = ask(L"Show value", L"Enter a variable name or C++ expression to display.", edited.value); break;
        case BlockType::ActionReadNumber:
            ok = ask(L"Read number", L"Name of an existing number variable to fill from input.", edited.value); break;
        case BlockType::ActionChangeNumber:
            ok = ask(L"Change number", L"Name of an existing number variable.", edited.value) &&
                 ask(L"Amount to add", L"Number to add (use a negative number to subtract).", edited.extra); break;
        case BlockType::ActionComment:
            ok = ask(L"Comment", L"Write a note for someone reading the code.", edited.value); break;
        default:
            a.status = "This syntax tile is fixed. Double-click Name, Type name, Expression, Number, Text, Character, C++ keyword, or Symbol tiles to edit their values.";
            InvalidateRect(a.hwnd, nullptr, FALSE);
            return false;
    }
    if (!ok) { a.status = "Edit cancelled; your block was not changed."; InvalidateRect(a.hwnd, nullptr, FALSE); return false; }
    a.remember();
    a.blocks[index] = std::move(edited);
    a.selected = index;
    a.status = "Block updated. Predict what will change, then press Run.";
    refresh_and_draw(a);
    return true;
}

void move_block(App& a, int direction) {
    const int i = a.selected;
    if (i < 0 || i >= static_cast<int>(a.blocks.size())) return;
    if (atomic_canvas_mode(a.blocks)) {
        Block& selected = a.blocks[static_cast<size_t>(i)];
        const int nextRow = std::max(0, selected.gridY + direction);
        if (nextRow == selected.gridY) return;
        const bool action = cpplab::is_atomic_action_type(selected.type);
        const bool rowHasAction = std::any_of(a.blocks.begin(), a.blocks.end(), [&](const Block& b) {
            return &b != &selected && b.gridY == nextRow && cpplab::is_atomic_action_type(b.type);
        });
        const bool rowHasAnything = std::any_of(a.blocks.begin(), a.blocks.end(), [&](const Block& b) {
            return &b != &selected && b.gridY == nextRow;
        });
        a.remember();
        if ((action && rowHasAnything) || (!action && rowHasAction)) {
            // Move the other source lines down so helper actions never share a row with tokens.
            for (auto& b : a.blocks) if (&b != &selected && b.gridY >= nextRow) ++b.gridY;
        } else {
            auto occupied = std::find_if(a.blocks.begin(), a.blocks.end(), [&](const Block& b) {
                return &b != &selected && b.gridY == nextRow && b.gridX == selected.gridX;
            });
            if (occupied != a.blocks.end()) occupied->gridY = selected.gridY;
        }
        selected.gridX = action ? 0 : selected.gridX;
        selected.gridY = nextRow;
        a.status = "Moved tile to grid row " + std::to_string(nextRow + 1) + ". A helper action always occupies its own source line.";
        refresh_and_draw(a);
        return;
    }
    int j = i + direction;
    if (j < 0 || j >= static_cast<int>(a.blocks.size())) return;
    a.remember();
    std::swap(a.blocks[i], a.blocks[j]);
    a.selected = j;
    a.status = "Block moved. Check its indentation, then Run to see what happens.";
    refresh_and_draw(a);
}
void indent_block(App& a, int delta) {
    if (a.selected < 0 || a.selected >= static_cast<int>(a.blocks.size())) return;
    if (atomic_canvas_mode(a.blocks)) {
        Block& selected = a.blocks[static_cast<size_t>(a.selected)];
        if (cpplab::is_atomic_action_type(selected.type)) {
            a.status = "Set number, Print text, and other helper actions each occupy their own line; braces determine indentation.";
            InvalidateRect(a.hwnd, nullptr, FALSE);
            return;
        }
        const int maxCol = 1000;
        const int nextCol = std::clamp(selected.gridX + delta, 0, maxCol);
        if (nextCol == selected.gridX) return;
        a.remember();
        auto occupied = std::find_if(a.blocks.begin(), a.blocks.end(), [&](const Block& b) {
            return &b != &selected && b.gridY == selected.gridY && b.gridX == nextCol;
        });
        if (occupied != a.blocks.end()) occupied->gridX = selected.gridX;
        selected.gridX = nextCol;
        a.status = "Moved tile to grid column " + std::to_string(nextCol + 1) + ". Left-to-right order builds the C++ line.";
        refresh_and_draw(a);
        return;
    }
    int current = a.blocks[a.selected].depth;
    int next = std::clamp(current + delta, 0, 6);
    if (next == current) return;
    a.remember();
    a.blocks[a.selected].depth = next;
    a.status = delta > 0 ? "Indented: this block will be inside the previous open scope if the levels match." : "Outdented: this block moves toward the left edge.";
    refresh_and_draw(a);
}

void undo(App& a) {
    if (a.undoStack.empty()) { a.status = "Nothing to undo yet."; return; }
    UndoState previous = std::move(a.undoStack.back()); a.undoStack.pop_back();
    a.blocks = std::move(previous.blocks);
    a.importedSource = std::move(previous.importedSource);
    a.importedSourceActive = previous.importedSourceActive;
    a.selected = a.blocks.empty() ? -1 : std::clamp(a.selected, 0, static_cast<int>(a.blocks.size()) - 1);
    a.status = "Undid the last change.";
    refresh_and_draw(a, a.importedSourceActive);
}
void remove_selected(App& a) {
    if (a.selected < 0 || a.selected >= static_cast<int>(a.blocks.size())) return;
    a.remember();
    a.blocks.erase(a.blocks.begin() + a.selected);
    a.selected = a.blocks.empty() ? -1 : std::min(a.selected, static_cast<int>(a.blocks.size()) - 1);
    a.status = "Removed the selected block. Run to see whether the program still does what you expect.";
    refresh_and_draw(a);
}

void save_code(App& a) {
    a.refresh_code();
    wchar_t filename[MAX_PATH * 4] = L"my_cpp_program.cpp";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = a.hwnd;
    ofn.lpstrFilter = L"C++ source files (*.cpp)\0*.cpp\0All files (*.*)\0*.*\0\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = static_cast<DWORD>(std::size(filename));
    ofn.lpstrDefExt = L"cpp";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetSaveFileNameW(&ofn)) { a.status = "Save cancelled."; return; }
    std::ofstream file(std::filesystem::path(filename), std::ios::binary);
    if (!file) { a.status = "Could not save that file. Try a folder where you have write access."; return; }
    file << a.generated.code;
    a.status = "Saved real C++ source to: " + to_utf8(filename);
    InvalidateRect(a.hwnd, nullptr, FALSE);
}

void load_cpp(App& a) {
    wchar_t filename[MAX_PATH * 4]{};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = a.hwnd;
    ofn.lpstrFilter = L"C++ source files (*.cpp;*.cc;*.cxx;*.h;*.hpp)\0*.cpp;*.cc;*.cxx;*.h;*.hpp\0All files (*.*)\0*.*\0\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = static_cast<DWORD>(std::size(filename));
    ofn.lpstrTitle = L"Load C++ source into the block grid";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
    if (!GetOpenFileNameW(&ofn)) { a.status = "Load cancelled."; InvalidateRect(a.hwnd, nullptr, FALSE); return; }

    std::ifstream file(std::filesystem::path(filename), std::ios::binary);
    if (!file) { a.status = "Could not open that C++ file."; InvalidateRect(a.hwnd, nullptr, FALSE); return; }
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (file.bad()) { a.status = "A read error occurred while loading that C++ file."; InvalidateRect(a.hwnd, nullptr, FALSE); return; }
    constexpr size_t MAX_IMPORT_BYTES = 1024 * 1024;
    if (source.size() > MAX_IMPORT_BYTES) {
        MessageBoxW(a.hwnd, L"This learning editor limits imported files to 1 MB so thousands of tiles do not freeze the window. Split a large project into smaller source files and load one at a time.",
            L"C++ file is too large", MB_OK | MB_ICONWARNING);
        return;
    }
    if (!a.blocks.empty() || a.importedSourceActive) {
        int answer = MessageBoxW(a.hwnd,
            L"Load this file into the visual grid? This replaces the blocks currently in the workspace. Undo can restore them.",
            L"Replace the current program?", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
        if (answer != IDYES) { a.status = "Load cancelled; current blocks were kept."; InvalidateRect(a.hwnd, nullptr, FALSE); return; }
    }
    std::vector<Block> parsed = cpplab::tokenize_cpp_source(source, 20000);
    size_t firstContent = 0;
    if (source.size() >= 3 && static_cast<unsigned char>(source[0]) == 0xEF &&
        static_cast<unsigned char>(source[1]) == 0xBB && static_cast<unsigned char>(source[2]) == 0xBF) firstContent = 3;
    const bool hasNonWhitespace = std::any_of(source.begin() + static_cast<std::ptrdiff_t>(firstContent), source.end(),
        [](unsigned char ch) { return std::isspace(ch) == 0; });
    if (hasNonWhitespace && parsed.empty()) {
        MessageBoxW(a.hwnd, L"This file has more than 20,000 tokens or could not be represented in the visual grid. The current program was not changed.",
            L"Could not create the block view", MB_OK | MB_ICONWARNING);
        return;
    }
    a.remember();
    a.blocks = std::move(parsed);
    a.importedSource = std::move(source);
    a.importedSourceActive = true;
    a.selected = -1;
    a.handBlockType = -1; a.handSticky = false;
    a.canvasScroll = 0; a.canvasScrollX = 0; a.codeScroll = 0;
    a.status = "Loaded C++ source into individual grid tiles. The code panel preserves the original file until you edit the blocks.";
    refresh_and_draw(a, true);
}

void clear_all_blocks(App& a) {
    const ULONGLONG now = GetTickCount64();
    if (now > a.clearAllArmedUntil) {
        a.clearAllArmedUntil = now + 2000;
        a.status = "Click again in quick succession to clear all the blocks";
        InvalidateRect(a.hwnd, nullptr, FALSE);
        return;
    }
    a.clearAllArmedUntil = 0;
    if (a.blocks.empty() && !a.importedSourceActive) {
        a.status = "There are no blocks to clear.";
        InvalidateRect(a.hwnd, nullptr, FALSE);
        return;
    }
    a.remember();
    a.blocks.clear();
    a.selected = -1;
    a.handBlockType = -1; a.handSticky = false;
    a.canvasScroll = 0; a.canvasScrollX = 0; a.codeScroll = 0;
    a.status = "Cleared all blocks. Use Undo to restore them.";
    refresh_and_draw(a);
}

std::string friendly_compile_error(const std::string& compilerText, const cpplab::GenerateResult& gen, const std::vector<Block>& blocks, int& blockIndex) {
    blockIndex = -1;
    std::regex gppLine(":([0-9]+):([0-9]+):\\s*(fatal error|error|warning):");
    std::regex clLine("\\(([0-9]+)\\)\\s*:\\s*(fatal error|error|warning)");
    std::smatch m;
    if (std::regex_search(compilerText, m, gppLine) || std::regex_search(compilerText, m, clLine)) {
        int lineNo = 0;
        try { lineNo = std::stoi(m[1].str()); } catch (...) { lineNo = 0; }
        if (lineNo > 0 && lineNo <= static_cast<int>(gen.lineOwners.size())) blockIndex = gen.lineOwners[static_cast<size_t>(lineNo - 1)];
    }
    std::string text = "The C++ compiler found a problem. That is normal while learning.\nRead the first error, then edit the block nearest to it.\n";
    if (blockIndex >= 0 && blockIndex < static_cast<int>(blocks.size())) text += "Likely location: block " + std::to_string(blockIndex + 1) + " (" + cpplab::block_name(blocks[blockIndex].type) + ").\n";
    text += "\nCOMPILER MESSAGE:\n" + compilerText;
    return text;
}

void run_program(App& a) {
    a.refresh_code();
    if (!a.generated.errors.empty()) {
        std::ostringstream out;
        out << "[ERROR] Fix the block layout before compiling:\n\n";
        for (const auto& e : a.generated.errors) out << "• " << e << "\n";
        out << "\nTip: If a block should be inside If/Repeat/While, indent it one level. Otherwise use Outdent -.";
        std::smatch layoutMatch;
        std::regex blockNumber("Block ([0-9]+)");
        if (std::regex_search(a.generated.errors.front(), layoutMatch, blockNumber)) {
            try {
                int badBlock = std::stoi(layoutMatch[1].str()) - 1;
                if (badBlock >= 0 && badBlock < static_cast<int>(a.blocks.size())) a.selected = badBlock;
            } catch (...) {}
        }
        a.lastRunBlock = -1;
        set_output(a, out.str());
        a.status = "The block layout needs attention; nothing was compiled.";
        return;
    }
    if (a.compiler.path.empty()) {
        set_output(a, "[ERROR] No C++ compiler was found.\n\nThe app itself does not include a compiler. Run Start_Cpp_Circuit_Lab.bat and enter your MSYS2 installation folder when asked, or make g++ available on PATH.\n\nYou can also launch the app from a Visual Studio Developer Command Prompt if C++ Build Tools are installed.\n\nThe code preview and editor still work without a compiler.");
        a.status = "Run needs g++/clang++ on PATH or an active Visual Studio C++ environment.";
        return;
    }

    const auto source = a.tempDir / L"cpp_circuit_lab_program.cpp";
    const auto exe = a.tempDir / L"cpp_circuit_lab_program.exe";
    std::error_code ec;
    std::filesystem::create_directories(a.tempDir, ec);
    {
        std::ofstream file(source, std::ios::binary);
        if (!file) { set_output(a, "[ERROR] Could not write the temporary C++ source. Check write permissions for your Windows Temp folder."); return; }
        file << a.generated.code;
    }

    std::wstring args;
    auto sourcePath = source.wstring();
    auto exePath = exe.wstring();
    if (a.compiler.kind == Compiler::Kind::Msvc) {
        auto obj = (a.tempDir / L"cpp_circuit_lab_program.obj").wstring();
        args = L"/nologo /EHsc /std:c++17 /utf-8 " + quote_arg(sourcePath) + L" /Fo:" + quote_arg(obj) + L" /Fe:" + quote_arg(exePath);
    } else {
        args = L"-std=c++17 -Wall -Wextra -pedantic -O0 " + quote_arg(sourcePath) + L" -o " + quote_arg(exePath);
    }
    a.status = "Compiling your generated C++ with " + a.compiler.name + "...";
    InvalidateRect(a.hwnd, nullptr, FALSE); UpdateWindow(a.hwnd);
    auto compile = run_process(a.compiler.path, args, 15000);
    if (compile.timedOut) {
        set_output(a, "[TOOLCHAIN] " + to_utf8(a.compiler.path) + "\n\n[BUILD] Compiling your generated C++...\n[ERROR] Compilation took longer than 15 seconds and was stopped.\n" + compile.output);
        a.status = "Compilation timed out.";
        return;
    }
    if (compile.exitCode != 0) {
        int blockIndex = -1;
        std::string feedback = "[TOOLCHAIN] " + to_utf8(a.compiler.path) +
            "\n\n[BUILD] Compiling your generated C++...\n[BUILD] Compilation failed.\n\n" +
            friendly_compile_error(compile.output, a.generated, a.blocks, blockIndex);
        a.lastRunBlock = blockIndex;
        set_output(a, feedback);
        if (blockIndex >= 0 && blockIndex < static_cast<int>(a.blocks.size())) a.selected = blockIndex;
        a.status = "It did not compile yet. Read the first error; then experiment with the indicated block.";
        InvalidateRect(a.hwnd, nullptr, FALSE);
        return;
    }

    std::string programInput;
    const bool needsInput = std::any_of(a.blocks.begin(), a.blocks.end(), [](const Block& b) { return b.type == BlockType::ReadInput; }) ||
        a.generated.code.find("std::cin") != std::string::npos || a.generated.code.find("cin >>") != std::string::npos;
    if (needsInput) {
        std::string inputPrompt = "Enter the input values your program should read. Put each value on its own line. For example, 12 then Enter. The values are sent to std::cin when the program starts.";
        std::string defaultInput;
        for (const auto& b : a.blocks) if (b.type == BlockType::ReadInput) defaultInput += "10\r\n";
        if (!prompt_input(a.hwnd, L"Program input", to_wide(inputPrompt).c_str(), defaultInput, programInput, true)) {
            a.status = "Run cancelled before the program started.";
            InvalidateRect(a.hwnd, nullptr, FALSE);
            return;
        }
    }
    a.status = "Compiled successfully. Running your program (maximum 3 seconds)...";
    InvalidateRect(a.hwnd, nullptr, FALSE); UpdateWindow(a.hwnd);
    auto run = run_process(exe, L"", 3000, programInput);
    std::ostringstream out;
    out << "[TOOLCHAIN] " << to_utf8(a.compiler.path) << "\n\n";
    out << "[BUILD] Compiling generated C++ with " << a.compiler.name << "...\n";
    out << "[BUILD] Compilation succeeded.\n\n";
    out << "[RUN] Launching the compiled program (3-second limit)...\n\n";
    out << "[PROGRAM OUTPUT]\n";
    if (run.output.empty()) out << "(The program ran without printing anything.)\n";
    else out << run.output;
    if (run.timedOut) out << "\nThe program was stopped after the time limit. Check for an endless loop.\n";
    else if (run.exitCode != 0) out << "\n[ERROR] Program exit code: " << run.exitCode << ". The program stopped with a runtime problem.\n";
    else out << "\n[Finished] Exit code 0. Change one block and run it again.\n";
    if (!compile.output.empty()) out << "\nCompiler notes:\n" << compile.output;
    set_output(a, out.str());
    a.lastRunBlock = -1;
    a.status = run.timedOut ? "Run stopped for taking too long. Read the While block and try again." : "Run complete. Change one thing, predict the result, and try again.";
    InvalidateRect(a.hwnd, nullptr, FALSE);
}

void perform_action(App& a, int action) {
    if (action != A_CLEAR_ALL) a.clearAllArmedUntil = 0;
    switch (action) {
        case A_RUN: run_program(a); break;
        case A_UNDO: undo(a); break;
        case A_EDIT: edit_block(a, a.selected); break;
        case A_DELETE: remove_selected(a); break;
        case A_UP: move_block(a, -1); break;
        case A_DOWN: move_block(a, 1); break;
        case A_INDENT: indent_block(a, 1); break;
        case A_OUTDENT: indent_block(a, -1); break;
        case A_EXAMPLE:
            a.remember(); a.blocks = cpplab::atomic_example_program(); a.selected = -1; a.handBlockType = -1; a.handSticky = false; a.canvasScroll = 0;
            a.lastRunBlock = -1; a.status = "Loaded the starter experiment. Change one block and predict the result."; refresh_and_draw(a); break;
        case A_GLOSSARY: a.overlayGlossary = true; a.overlayHelp = false; a.glossaryScroll = 0; InvalidateRect(a.hwnd, nullptr, FALSE); break;
        case A_HELP: a.overlayHelp = true; a.overlayGlossary = false; InvalidateRect(a.hwnd, nullptr, FALSE); break;
        case A_SAVE: save_code(a); break;
        case A_LOAD_CPP: load_cpp(a); break;
        case A_CLEAR_ALL: clear_all_blocks(a); break;
        case A_CLOSE_OVERLAY: a.overlayGlossary = false; a.overlayHelp = false; InvalidateRect(a.hwnd, nullptr, FALSE); break;
    }
}

Hit* hit_test(App& a, int x, int y) {
    for (auto it = a.hits.rbegin(); it != a.hits.rend(); ++it) if (contains(it->rect, x, y)) return &*it;
    return nullptr;
}
void scroll_to_track(App& a, App::ScrollKind kind, const Hit& hit, int y, bool dragging) {
    RECT rc{}; GetClientRect(a.hwnd, &rc); int h = rc.bottom, bodyTop = 78, bodyH = h - 78 - 31 - 5;
    int w = rc.right, centerX = 240, centerW = std::max(370, static_cast<int>((w - 260) * 0.43));
    int rightX = centerX + centerW + 10; int rightW = w - rightX - 12;
    int split = bodyTop + bodyH * 57 / 100;
    int trackTop = hit.rect.y;
    int trackH = hit.rect.h;
    int maxOffset = 0, thumbH = 28;
    if (kind == App::ScrollKind::Palette) {
        int viewH = std::max(1, bodyH - 58 - 76);
        int rows = (static_cast<int>(cpplab::block_catalog().size()) + PALETTE_COLUMNS - 1) / PALETTE_COLUMNS;
        int total = rows * PALETTE_ROW_HEIGHT;
        maxOffset = std::max(0, total - viewH); thumbH = std::max(28, trackH * viewH / std::max(total, viewH));
    } else if (kind == App::ScrollKind::Canvas) {
        int viewH = bodyH - 72; int total = canvas_content_height(a.blocks);
        maxOffset = std::max(0, total - viewH); thumbH = std::max(28, viewH * viewH / std::max(total, viewH));
    } else if (kind == App::ScrollKind::Code) {
        int viewH = split - bodyTop - 7 - 66; maxOffset = std::max(0, static_cast<int>(a.codeLines.size()) - std::max(1, viewH / 19));
        thumbH = std::max(28, trackH * std::max(1, viewH / 19) / std::max(1, static_cast<int>(a.codeLines.size())));
    } else if (kind == App::ScrollKind::Output) {
        int viewH = bodyTop + bodyH - split - 2 - 67; auto lines = wrap_output(a.output, std::max(24, (rightW - 48) / 7));
        maxOffset = std::max(0, static_cast<int>(lines.size()) - std::max(1, viewH / 19));
        thumbH = std::max(28, trackH * std::max(1, viewH / 19) / std::max(1, static_cast<int>(lines.size())));
    } else if (kind == App::ScrollKind::Glossary) {
        int viewH = std::max(1, h - 80 - 98); int total = static_cast<int>(cpplab::block_catalog().size()) * GLOSSARY_ROW_HEIGHT;
        maxOffset = std::max(0, total - viewH); thumbH = std::max(28, trackH * viewH / std::max(total, viewH));
    }
    thumbH = std::min(thumbH, trackH);
    int travel = std::max(0, trackH - thumbH);
    int newOffset = 0;
    if (dragging) {
        int desiredTop = y - a.scrollGrabY - trackTop;
        desiredTop = std::clamp(desiredTop, 0, travel);
        if (travel > 0) newOffset = static_cast<int>((static_cast<long long>(desiredTop) * maxOffset) / travel);
    } else {
        int desiredTop = y - trackTop - thumbH / 2;
        desiredTop = std::clamp(desiredTop, 0, travel);
        if (travel > 0) newOffset = static_cast<int>((static_cast<long long>(desiredTop) * maxOffset) / travel);
    }
    switch (kind) {
        case App::ScrollKind::Palette: a.paletteScroll = newOffset; break;
        case App::ScrollKind::Canvas: a.canvasScroll = newOffset; break;
        case App::ScrollKind::Code: a.codeScroll = newOffset; break;
        case App::ScrollKind::Output: a.outputScroll = newOffset; break;
        case App::ScrollKind::Glossary: a.glossaryScroll = newOffset; break;
        case App::ScrollKind::None: break;
    }
    a.clamp_scrolls();
    InvalidateRect(a.hwnd, nullptr, FALSE);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* app = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        app = static_cast<App*>(cs->lpCreateParams);
        app->hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    switch (msg) {
        case WM_CREATE:
            app->font = CreateFontW(-17, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            app->smallFont = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            app->monoFont = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
            app->titleFont = CreateFontW(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            app->tileFont = CreateFontW(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            return 0;
        case WM_ERASEBKGND: return 1;
        case WM_PAINT: { PAINTSTRUCT ps{}; BeginPaint(hwnd, &ps); paint(*app); EndPaint(hwnd, &ps); return 0; }
        case WM_SIZE: app->clamp_scrolls(); InvalidateRect(hwnd, nullptr, FALSE); return 0;
        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp); app->mouseX = x; app->mouseY = y;
            if (app->scrollDrag != App::ScrollKind::None) {
                Hit fake{Hit::Kind::ScrollThumb, {0, 0, 0, 0}, 0, app->scrollDrag};
                // Recover the current track geometry from the hit map created by the last paint.
                for (const auto& h : app->hits) if (h.kind == Hit::Kind::ScrollTrack && h.scrollKind == app->scrollDrag) { fake.rect = h.rect; break; }
                scroll_to_track(*app, app->scrollDrag, fake, y, true);
                return 0;
            }
            if (app->dragSource >= 0 && (std::abs(y - app->dragStartY) > 5 || std::abs(x - app->dragStartX) > 5)) {
                app->status = "Drag preview: release to snap the square tile to a grid cell.";
                InvalidateRect(hwnd, nullptr, FALSE);
            } else if (app->paletteDragType >= 0 &&
                       (std::abs(y - app->paletteDragStartY) > 5 || std::abs(x - app->paletteDragStartX) > 5)) {
                app->status = "Drag preview: drop this square tile onto your program grid.";
                InvalidateRect(hwnd, nullptr, FALSE);
            } else {
                int oldPal = app->hoveredPalette;
                int oldCanvas = app->hoveredCanvas;
                app->hoveredPalette = -1;
                app->hoveredCanvas = -1;
                for (const auto& h : app->hits) {
                    if (!contains(h.rect, x, y)) continue;
                    if (h.kind == Hit::Kind::Palette) app->hoveredPalette = h.value;
                    else if (h.kind == Hit::Kind::CanvasBlock) app->hoveredCanvas = h.value;
                }
                const bool overCanvas = contains(canvas_view_for(hwnd), x, y);
                SetCursor(LoadCursorW(nullptr, (app->hoveredPalette >= 0 || app->hoveredCanvas >= 0 ||
                    (app->handBlockType >= 0 && overCanvas)) ? IDC_HAND : IDC_ARROW));
                (void)oldPal; (void)oldCanvas;
                InvalidateRect(hwnd, nullptr, FALSE); // keep the compact hover hint near the pointer
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
            SetCapture(hwnd);
            Hit* hit = hit_test(*app, x, y);
            if (!(hit && hit->kind == Hit::Kind::Button && hit->value == A_CLEAR_ALL)) app->clearAllArmedUntil = 0;
            if (app->overlayGlossary || app->overlayHelp) {
                if (!hit) return 0;
                bool allowed = hit->kind == Hit::Kind::Button && hit->value == A_CLOSE_OVERLAY;
                allowed = allowed || (app->overlayGlossary &&
                    (hit->kind == Hit::Kind::ScrollTrack || hit->kind == Hit::Kind::ScrollThumb) &&
                    hit->scrollKind == App::ScrollKind::Glossary);
                if (!allowed) return 0;
            }
            const Rect canvasView = canvas_view_for(hwnd);
            if (app->handBlockType >= 0 && contains(canvasView, x, y) &&
                (!hit || hit->kind == Hit::Kind::CanvasBlock)) {
                const int maxRow = std::max(canvas_max_grid_row(app->blocks), (app->canvasScroll + canvasView.h) / GRID_CELL);
                const int row = grid_row_from_point(canvasView, y, app->canvasScroll, maxRow);
                const int col = grid_column_from_point(canvasView, x, app->canvasScrollX, canvas_max_grid_column(app->blocks));
                place_block_at_grid(*app, static_cast<BlockType>(app->handBlockType), row, col);
                return 0;
            }
            if (!hit) return 0;
            if (hit->kind == Hit::Kind::Button) { ReleaseCapture(); perform_action(*app, hit->value); return 0; }
            if (hit->kind == Hit::Kind::Palette) {
                app->paletteDragType = hit->value;
                app->paletteDragStartX = x; app->paletteDragStartY = y;
                app->paletteDragGrabX = x - hit->rect.x; app->paletteDragGrabY = y - hit->rect.y;
                app->paletteDragKeep = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                app->dragStartX = x; app->dragStartY = y;
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }
            if (hit->kind == Hit::Kind::CanvasBlock) {
                app->selected = hit->value; app->dragSource = hit->value; app->dragStartX = x; app->dragStartY = y;
                if (hit->value >= 0 && hit->value < static_cast<int>(app->blockRects.size())) {
                    app->dragGrabX = x - app->blockRects[static_cast<size_t>(hit->value)].x;
                    app->dragGrabY = y - app->blockRects[static_cast<size_t>(hit->value)].y;
                } else { app->dragGrabX = 0; app->dragGrabY = 0; }
                app->status = "Selected " + std::string(cpplab::block_name(app->blocks[app->selected].type)) + ". Drag the square tile to a new grid cell, or double-click to edit.";
                InvalidateRect(hwnd, nullptr, FALSE); return 0;
            }
            if (hit->kind == Hit::Kind::ScrollThumb) {
                app->scrollDrag = hit->scrollKind; app->scrollGrabY = y - hit->rect.y; return 0;
            }
            if (hit->kind == Hit::Kind::ScrollTrack) {
                app->scrollDrag = hit->scrollKind; app->scrollGrabY = 0;
                scroll_to_track(*app, hit->scrollKind, *hit, y, false); return 0;
            }
            if (hit->kind == Hit::Kind::OverlayClose) { perform_action(*app, A_CLOSE_OVERLAY); return 0; }
            return 0;
        }
        case WM_LBUTTONDBLCLK: {
            int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
            Hit* hit = hit_test(*app, x, y);
            if (hit && hit->kind == Hit::Kind::CanvasBlock) { ReleaseCapture(); app->selected = hit->value; app->dragSource = -1; edit_block(*app, app->selected); }
            // Palette clicks pick a tile up; double-clicking a program tile edits its values.
            return 0;
        }
        case WM_LBUTTONUP: {
            int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
            if (app->scrollDrag != App::ScrollKind::None) app->scrollDrag = App::ScrollKind::None;
            else if (app->dragSource >= 0) {
                int source = app->dragSource;
                const bool moved = std::abs(y - app->dragStartY) > 10 || std::abs(x - app->dragStartX) > 10;
                const Rect canvasView = canvas_view_for(hwnd);
                if (moved && contains(canvasView, x, y)) {
                    const int viewX = canvasView.x;
                    const int viewY = canvasView.y;
                    const bool atomicMode = atomic_canvas_mode(app->blocks);
                    const int blockLeft = x - app->dragGrabX;
                    const int blockTop = y - app->dragGrabY;
                    int target = static_cast<int>(std::lround((blockTop - (viewY + BLOCK_ORIGIN_INSET) + app->canvasScroll) / static_cast<double>(GRID_CELL)));
                    target = std::clamp(target, 0, std::max(canvas_max_grid_row(app->blocks), (app->canvasScroll + canvasView.h) / GRID_CELL));
                    const Block& originalBlock = app->blocks[static_cast<size_t>(source)];
                    const int newDepth = cpplab::is_atomic_action_type(originalBlock.type) ? 0 : std::clamp(static_cast<int>(std::lround(
                        (blockLeft - (viewX + BLOCK_ORIGIN_INSET) + app->canvasScrollX) / static_cast<double>(GRID_CELL))), 0,
                        std::max(canvas_max_grid_column(app->blocks), (app->canvasScrollX + canvasView.w) / GRID_CELL));
                    const bool changed = atomicMode
                        ? (app->blocks[static_cast<size_t>(source)].gridY != target || app->blocks[static_cast<size_t>(source)].gridX != newDepth)
                        : (target != source || newDepth != app->blocks[static_cast<size_t>(source)].depth);
                    if (changed) {
                        app->remember();
                        if (atomicMode) {
                            Block& moving = app->blocks[static_cast<size_t>(source)];
                            const bool movingAction = cpplab::is_atomic_action_type(moving.type);
                            const bool rowHasAction = std::any_of(app->blocks.begin(), app->blocks.end(), [&](const Block& b) {
                                return &b != &moving && b.gridY == target && cpplab::is_atomic_action_type(b.type);
                            });
                            const bool rowHasAnything = std::any_of(app->blocks.begin(), app->blocks.end(), [&](const Block& b) {
                                return &b != &moving && b.gridY == target;
                            });
                            if ((movingAction && rowHasAnything) || (!movingAction && rowHasAction)) {
                                for (auto& b : app->blocks)
                                    if (&b != &moving && b.gridY >= target) ++b.gridY;
                            } else {
                                bool occupied = false;
                                for (size_t k = 0; k < app->blocks.size(); ++k)
                                    if (static_cast<int>(k) != source && app->blocks[k].gridY == target && app->blocks[k].gridX == newDepth) occupied = true;
                                if (occupied) {
                                    std::vector<size_t> shift;
                                    for (size_t k = 0; k < app->blocks.size(); ++k)
                                        if (static_cast<int>(k) != source && app->blocks[k].gridY == target && app->blocks[k].gridX >= newDepth) shift.push_back(k);
                                    std::sort(shift.begin(), shift.end(), [&](size_t left, size_t right) { return app->blocks[left].gridX > app->blocks[right].gridX; });
                                    for (size_t k : shift) ++app->blocks[k].gridX;
                                }
                            }
                            moving.gridX = movingAction ? 0 : newDepth;
                            moving.gridY = target; moving.depth = 0;
                            app->selected = source;
                        } else {
                            Block block = app->blocks[static_cast<size_t>(source)];
                            app->blocks.erase(app->blocks.begin() + source);
                            target = std::clamp(target, 0, static_cast<int>(app->blocks.size()));
                            block.depth = newDepth;
                            app->blocks.insert(app->blocks.begin() + target, block);
                            app->selected = target;
                        }
                        app->status = "Tile placed at column " + std::to_string(newDepth) + ", row " + std::to_string(target + 1) + ". Syntax tiles are individual words/symbols; value tiles are editable slots; action tiles are optional shortcuts.";
                        refresh_and_draw(*app);
                    }
                } else if (moved) {
                    app->status = "Tile stayed in place. Drop a program tile inside the grid to move it.";
                }
                app->dragSource = -1;
            } else if (app->paletteDragType >= 0) {
                const BlockType type = static_cast<BlockType>(app->paletteDragType);
                const bool moved = std::abs(x - app->paletteDragStartX) > 5 || std::abs(y - app->paletteDragStartY) > 5;
                if (!moved) {
                    // A click picks up a tile; Shift-click keeps that tile in the hand after placement.
                    app->handBlockType = static_cast<int>(type);
                    app->handSticky = app->paletteDragKeep;
                    app->selected = -1;
                    app->status = "Hand: " + std::string(cpplab::block_name(type)) +
                        (app->handSticky ? " (kept). Click grid cells to place copies." :
                                           ". Click one grid cell to place it; the hand empties afterward.");
                } else {
                    const Rect view = canvas_view_for(hwnd);
                    if (contains(view, x, y)) {
                        const int tileLeft = x - app->paletteDragGrabX;
                        const int tileTop = y - app->paletteDragGrabY;
                        int target = static_cast<int>(std::lround(
                            (tileTop - (view.y + BLOCK_ORIGIN_INSET) + app->canvasScroll) / static_cast<double>(GRID_CELL)));
                        target = std::clamp(target, 0, std::max(canvas_max_grid_row(app->blocks), (app->canvasScroll + view.h) / GRID_CELL));
                        const int depth = cpplab::is_atomic_action_type(type) ? 0 : std::clamp(static_cast<int>(std::lround(
                            (tileLeft - (view.x + BLOCK_ORIGIN_INSET) + app->canvasScrollX) / static_cast<double>(GRID_CELL))), 0,
                            std::max(canvas_max_grid_column(app->blocks), (app->canvasScrollX + view.w) / GRID_CELL));
                        const bool keepAfterDrop = app->paletteDragKeep;
                        // A direct drag acts like picking this tile up and placing it in one gesture.
                        app->handBlockType = static_cast<int>(type);
                        app->handSticky = keepAfterDrop;
                        const bool placed = place_block_at_grid(*app, type, target, depth);
                        if (!placed && keepAfterDrop) {
                            app->handBlockType = static_cast<int>(type);
                            app->handSticky = true;
                        }
                        if (placed && keepAfterDrop && type != BlockType::RawProgram) {
                            app->handBlockType = static_cast<int>(type);
                            app->handSticky = true;
                            app->status = "Placed tile. Hand kept: " + std::string(cpplab::tile_label(type)) + ". Click another cell to place a copy.";
                            InvalidateRect(hwnd, nullptr, FALSE);
                        }
                    } else if (app->paletteDragKeep) {
                        app->handBlockType = static_cast<int>(type);
                        app->handSticky = true;
                        app->status = "Hand kept: " + std::string(cpplab::block_name(type)) + ". Drop or click a grid cell to place it.";
                    } else {
                        app->status = "Tile not placed. Drop it inside the program grid, or click it to pick it up.";
                    }
                }
                app->paletteDragType = -1;
            }
            ReleaseCapture(); InvalidateRect(hwnd, nullptr, FALSE); return 0;
        }
        case WM_CAPTURECHANGED: app->dragSource = -1; app->paletteDragType = -1; app->scrollDrag = App::ScrollKind::None; return 0;
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA;
            POINT p{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}; ScreenToClient(hwnd, &p);
            if (app->overlayGlossary) app->glossaryScroll -= delta * 68;
            else if (!app->overlayHelp) {
                // Keep wheel movement local to the panel under the pointer.
                RECT rc{}; GetClientRect(hwnd, &rc); int bodyTop = 78, bodyH = rc.bottom - 78 - 31 - 5;
                int centerX = 240, centerW = std::max(370, static_cast<int>((rc.right - 260) * 0.43));
                int rightX = centerX + centerW + 10; int split = bodyTop + bodyH * 57 / 100;
                if (p.x < 232) app->paletteScroll -= delta * PALETTE_ROW_HEIGHT;
                else if (p.x >= centerX && p.x < rightX && (GetKeyState(VK_SHIFT) & 0x8000)) app->canvasScrollX -= delta * GRID_CELL;
                else if (p.x >= centerX && p.x < rightX) app->canvasScroll -= delta * CANVAS_PITCH;
                else if (p.x >= rightX && p.y < split) app->codeScroll -= delta * 3;
                else if (p.x >= rightX) app->outputScroll -= delta * 3;
            }
            app->clamp_scrolls(); InvalidateRect(hwnd, nullptr, FALSE); return 0;
        }
        case WM_KEYDOWN: {
            bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            if (app->overlayGlossary || app->overlayHelp) {
                if (wp == VK_ESCAPE || wp == VK_F1 || wp == 'G') { app->overlayGlossary = false; app->overlayHelp = false; InvalidateRect(hwnd, nullptr, FALSE); }
                else if (app->overlayGlossary && (wp == VK_DOWN || wp == VK_NEXT)) app->glossaryScroll += wp == VK_NEXT ? 450 : 60;
                else if (app->overlayGlossary && (wp == VK_UP || wp == VK_PRIOR)) app->glossaryScroll -= wp == VK_PRIOR ? 450 : 60;
                else if (app->overlayGlossary && wp == VK_HOME) app->glossaryScroll = 0;
                else if (app->overlayGlossary && wp == VK_END) app->glossaryScroll = 99999;
                app->clamp_scrolls(); InvalidateRect(hwnd, nullptr, FALSE); return 0;
            }
            if (wp == VK_F5) run_program(*app);
            else if (wp == VK_F1) { app->overlayHelp = true; app->overlayGlossary = false; InvalidateRect(hwnd, nullptr, FALSE); }
            else if (wp == 'G') { app->overlayGlossary = true; app->glossaryScroll = 0; InvalidateRect(hwnd, nullptr, FALSE); }
            else if (ctrl && wp == 'Z') undo(*app);
            else if (ctrl && wp == 'S') save_code(*app);
            else if (wp == VK_RETURN) edit_block(*app, app->selected);
            else if (wp == VK_DELETE) remove_selected(*app);
            else if (ctrl && wp == VK_UP) move_block(*app, -1);
            else if (ctrl && wp == VK_DOWN) move_block(*app, 1);
            else if (wp == VK_ESCAPE) {
                if (app->handBlockType >= 0) {
                    app->handBlockType = -1; app->handSticky = false;
                    app->status = "Hand: empty. Click a library tile to pick up a block.";
                } else {
                    app->status = "Tip: mistakes are useful. Change one block and press Run again.";
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }
        case WM_RBUTTONDOWN:
            if (app->handBlockType >= 0) {
                app->handBlockType = -1; app->handSticky = false;
                app->status = "Hand cleared. Right-click or press Esc to empty the hand.";
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case WM_GETMINMAXINFO: {
            auto* mmi = reinterpret_cast<MINMAXINFO*>(lp); mmi->ptMinTrackSize.x = 1200; mmi->ptMinTrackSize.y = 650; return 0;
        }
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCmd) {
    WNDCLASSW wc{};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = APP_CLASS;
    if (!RegisterClassW(&wc)) return 1;
    WNDCLASSW pwc{};
    pwc.lpfnWndProc = prompt_proc;
    pwc.hInstance = instance;
    pwc.hCursor = LoadCursorW(nullptr, IDC_IBEAM);
    pwc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    pwc.lpszClassName = PROMPT_CLASS;
    RegisterClassW(&pwc);

    App app;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int initialW = std::min(1480, std::max(1200, screenW - 48));
    int initialH = std::min(930, std::max(650, screenH - 90));
    HWND hwnd = CreateWindowExW(0, APP_CLASS, L"C++ Circuit Lab — Learn C++ by experimenting",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, initialW, initialH,
        nullptr, nullptr, instance, &app);
    if (!hwnd) return 1;
    ShowWindow(hwnd, showCmd);
    UpdateWindow(hwnd);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    return static_cast<int>(msg.wParam);
}

#else
#include <iostream>

int main(int argc, char** argv) {
    using namespace cpplab;
    if (argc > 1 && std::string(argv[1]) == "--self-test") {
        std::string report;
        bool ok = self_test(report);
        std::cout << report;
        return ok ? 0 : 1;
    }
    if (argc > 1 && std::string(argv[1]) == "--emit-demo") {
        auto generated = generate_cpp(atomic_example_program());
        if (!generated.errors.empty()) {
            for (const auto& e : generated.errors) std::cerr << e << '\n';
            return 1;
        }
        if (argc > 2) {
            std::ofstream file(argv[2], std::ios::binary);
            if (!file) { std::cerr << "Could not write output file.\n"; return 1; }
            file << generated.code;
        } else std::cout << generated.code;
        return 0;
    }
    std::cout << "C++ Circuit Lab is a native Windows desktop app.\n"
                 "This build is the testable C++ core. Use --self-test to validate code generation,\n"
                 "or --emit-demo [filename.cpp] to create the starter C++ program.\n";
    return 0;
}
#endif
