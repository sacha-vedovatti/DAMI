/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Tray System functions
*/

#include "TraySys.hpp"

TraySys::TraySys(Config &config, HINSTANCE instance) : _config(config), _gui(config), _instance(instance)
{
    tray = this;
}

TraySys::~TraySys()
{
    Shell_NotifyIconW(NIM_DELETE, &_nid);
    if (_hwnd)
        DestroyWindow(_hwnd);
    _gui.join();
    tray = nullptr;
}

bool TraySys::init(void)
{
    if (!_gui.init())
        return _error("GUI init failed");
    if (!_register())
        return false;
    _hwnd = CreateWindowExW(0, L"DAMI_TrayClass", L"DAMI", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, _instance, nullptr);
    if (!_hwnd)
        return _error("CreateWindow failed: ");

    HICON icon = (HICON) LoadImageW(nullptr, icon_path().c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE | LR_DEFAULTCOLOR);
    if (!icon)
        icon = LoadIconW(nullptr, MAKEINTRESOURCEW(IDI_APPLICATION));
    _nid = {};
    _nid.cbSize = sizeof(_nid);
    _nid.hWnd = _hwnd;
    _nid.uID = ID_TRAY_ICON;
    _nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    _nid.uCallbackMessage = WM_TRAY_ICON;
    _nid.hIcon = icon;
    wcsncpy_s(_nid.szTip, L"DAMI - Discord Apple Music Integration", _TRUNCATE);
    if (!Shell_NotifyIconW(NIM_ADD, &_nid))
        return _error("Shell_NotifyIcon failed: ");
    _nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &_nid);
    return true;
}

bool TraySys::_register(void)
{
    WNDCLASSEXW wc = {};

    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = _proc;
    wc.hInstance = _instance;
    wc.lpszClassName = L"DAMI_TrayClass";
    if (!RegisterClassExW(&wc))
        return _error("RegisterClassEx failed: ");
    return true;
}

int TraySys::run(void)
{
    MSG message;

    while (GetMessageW(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return (int) message.wParam;
}

void TraySys::set_tooltip(const std::wstring &text)
{
    _tooltip = text;
    wcsncpy_s(_nid.szTip, text.c_str(), _TRUNCATE);
    _nid.uFlags = NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &_nid);
}

void TraySys::_show_context_menu(void)
{
    HMENU menu = CreatePopupMenu();
    if (!menu)
        return;
    InsertMenuW(menu, 0, MF_BYPOSITION | MF_STRING, ID_MENU_SETTINGS, L"&Settings");
    InsertMenuW(menu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
    InsertMenuW(menu, 2, MF_BYPOSITION | MF_STRING, ID_MENU_QUIT,     L"E&xit");
    SetForegroundWindow(_hwnd);

    POINT pt;
    GetCursorPos(&pt);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_RIGHTALIGN, pt.x, pt.y, 0, _hwnd, nullptr);
    PostMessageW(_hwnd, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

LRESULT CALLBACK TraySys::_proc(HWND hwnd, UINT message, WPARAM param, LPARAM long_param)
{
    if (!tray)
        return DefWindowProcW(hwnd, message, param, long_param);
    switch (message) {
        case WM_TRAY_ICON:
            switch (LOWORD(long_param)) {
                case WM_LBUTTONUP:
                    tray->_gui.toggle();
                    break;
                case WM_RBUTTONUP:
                case NIN_KEYSELECT:
                    tray->_show_context_menu();
                    break;
                case WM_LBUTTONDBLCLK:
                    tray->_gui.toggle();;
                    break;
            }
            break;
        case WM_CONTEXTMENU:
            tray->_show_context_menu();
            break;
        case WM_COMMAND:
            switch (LOWORD(param)) {
                case ID_MENU_SETTINGS:
                    tray->_gui.toggle();
                    break;
                case ID_MENU_QUIT:
                    Shell_NotifyIconW(NIM_DELETE, &tray->_nid);
                    PostQuitMessage(0);
                    break;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hwnd, message, param, long_param);
    }
    return 0;
}

bool TraySys::_error(const std::string &err)
{
    std::cerr << "[TRAY] " << err << GetLastError() << std::endl;
    return false;
}
