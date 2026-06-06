/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Fenêtre de paramètres (Win32 natif, construite en code – pas de .rc)
*/

#include "Settings.hpp"


Settings::Settings(Config &config, HINSTANCE instance, HWND parent) : _config(config), _instance(instance), _parent(parent)
{
    settings = this;
}

void Settings::_register(void)
{
    
    WNDCLASSEXW wc = {};

    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = [](HWND h, UINT m, WPARAM w, LPARAM l) -> LRESULT {
        if (!settings)
            return DefWindowProcW(h, m, w, l);
        switch (m) {
            case WM_CREATE:
                settings->_on_init(h);
                return 0;
            case WM_COMMAND:
                if (LOWORD(w) == IDC_BTN_OK) {
                    settings->_on_ok(h);
                    DestroyWindow(h);
                    return 0;
                }
                if (LOWORD(w) == IDC_BTN_CANCEL) {
                    DestroyWindow(h);
                    return 0;
                }
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            default:
                return DefWindowProcW(h, m, w, l);
        }
    };
    wc.hInstance = _instance;
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"DAMI_SettingsClass";
    wc.hIcon = (HICON) LoadImageW(nullptr, icon_path().c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
    if (!wc.hIcon)
        wc.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(IDI_APPLICATION));
    RegisterClassExW(&wc);
}

void Settings::show(void)
{
    _register();

    const int W = 420, H = 350;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    HWND hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, L"DAMI_SettingsClass", L"DAMI - Settings", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        (screenW - W) / 2, (screenH - H) / 2, W, H, _parent, nullptr, _instance, nullptr);
    if (!hwnd) {
        std::cerr << "[GUI] CreateWindow failed: " << GetLastError() << std::endl;
        return;
    }
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0)) {
        if (!IsDialogMessageW(hwnd, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
}

static HWND makeLabel(HWND parent, HINSTANCE instance, int id, const wchar_t *text, int x, int y, int w, int h, bool bold = false)
{
    HWND label = CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT, x, y, w, h, parent, (HMENU) (intptr_t) id, instance, nullptr);

    if (bold) {
        HFONT font = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        SendMessageW(label, WM_SETFONT, (WPARAM) font, TRUE);
    } else {
        HFONT font = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        SendMessageW(label, WM_SETFONT, (WPARAM) font, TRUE);
    }
    return label;
}

static HWND makeCheck(HWND parent, HINSTANCE instance, int id, const wchar_t *text, int x, int y, int w, int h, bool checked)
{
    HWND check = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, x, y, w, h, parent, (HMENU) (intptr_t) id, instance, nullptr);
    HFONT font = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SendMessageW(check, WM_SETFONT, (WPARAM) font, TRUE);
    SendMessageW(check, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    return check;
}

static HWND makeButton(HWND parent, HINSTANCE instance, int id, const wchar_t *text, int x, int y, int w, int h)
{
    HWND button = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, x, y, w, h, parent, (HMENU) (intptr_t) id, instance, nullptr);
    HFONT font = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SendMessageW(button, WM_SETFONT, (WPARAM) font, TRUE);
    return button;
}

void Settings::_on_init(HWND hwnd)
{
    const int L = 20, CW = 360, CH = 22, GAP = 28;
    int y = 14;
    config_t *settings = _config.get_settings();

    makeLabel(hwnd, _instance, IDC_LBL_SECTION_RP, L"Rich Presence", L, y, CW, 20, true);
    y += 26;
    makeCheck(hwnd, _instance, IDC_CHK_TITLE, L"Show &title", L, y, CW, CH, settings->show_title);
    y += GAP;
    makeCheck(hwnd, _instance, IDC_CHK_ARTIST, L"Show &artist", L, y, CW, CH, settings->show_artist);
    y += GAP;
    makeCheck(hwnd, _instance, IDC_CHK_ALBUM, L"Show al&bum", L, y, CW, CH, settings->show_album);
    y += GAP;
    makeCheck(hwnd, _instance, IDC_CHK_COVER, L"Show &cover", L, y, CW, CH, settings->show_cover);
    y += GAP;
    makeCheck(hwnd, _instance, IDC_CHK_TIMESTAMPS, L"Show &timestamps", L, y, CW, CH, settings->show_timestamps);
    y += GAP + 8;
    CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, L, y, CW, 2, hwnd, nullptr, _instance, nullptr);
    y += 10;
    makeLabel(hwnd, _instance, IDC_LBL_SECTION_BEHAV, L"Behavior", L, y, CW, 20, true);
    y += 26;
    makeCheck(hwnd, _instance, IDC_CHK_AUTOSTART, L"Start with &Windows", L, y, CW, CH, settings->auto_start);
    y += GAP + 14;
    makeButton(hwnd, _instance, IDC_BTN_OK, L"&OK", 235, y, 75, 28);
    makeButton(hwnd, _instance, IDC_BTN_CANCEL, L"&Cancel", 320, y, 75, 28);
}

static bool is_checked(HWND parent, int id)
{
    HWND ctrl = GetDlgItem(parent, id);

    return ctrl && (SendMessageW(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void Settings::_on_ok(HWND hwnd)
{
    config_t *settings = _config.get_settings();

    settings->show_title = is_checked(hwnd, IDC_CHK_TITLE);
    settings->show_artist = is_checked(hwnd, IDC_CHK_ARTIST);
    settings->show_album = is_checked(hwnd, IDC_CHK_ALBUM);
    settings->show_cover = is_checked(hwnd, IDC_CHK_COVER);
    settings->show_timestamps = is_checked(hwnd, IDC_CHK_TIMESTAMPS);
    settings->auto_start = is_checked(hwnd, IDC_CHK_AUTOSTART);
    if (!_config.save())
        MessageBoxW(hwnd, L"Unable to save configuration.", L"Error", MB_OK | MB_ICONERROR);
    else
        std::cout << "[CONFIG] Settings saved." << std::endl;
}
