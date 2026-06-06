/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Tray System header file
*/

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <functional>

#include "../config/Config.hpp"

#define WM_TRAY_ICON (WM_USER + 1)
#define ID_TRAY_ICON 1001
#define ID_MENU_SETTINGS 2001
#define ID_MENU_QUIT 2002

class TraySys {
    public:
        TraySys(Config &config, HINSTANCE instance);
        ~TraySys();

        bool init(void);
        int  run(void);
        void set_tooltip(const std::wstring &text);

    private:
        bool _register(void);
        static LRESULT CALLBACK _proc(HWND hwnd, UINT message, WPARAM param, LPARAM long_param);
        void _show_context_menu(void);
        void _show_settings(void);
        void _update(void);
        bool _error(const std::string &err);

        Config &_config;
        HINSTANCE _instance;
        HWND _hwnd{nullptr};
        NOTIFYICONDATAW _nid{};
        std::wstring _tooltip;
};

static TraySys *tray = nullptr;
