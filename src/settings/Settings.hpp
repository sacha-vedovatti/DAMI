/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Window settings header file
*/

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <iostream>

#include "../config/Config.hpp"

#define IDC_CHK_TITLE        3001
#define IDC_CHK_ARTIST       3002
#define IDC_CHK_ALBUM        3003
#define IDC_CHK_COVER        3004
#define IDC_CHK_TIMESTAMPS   3005
#define IDC_CHK_AUTOSTART    3006
#define IDC_BTN_OK           IDOK
#define IDC_BTN_CANCEL       IDCANCEL
#define IDC_LBL_TITLE        3010
#define IDC_LBL_SECTION_RP   3011
#define IDC_LBL_SECTION_BEHAV 3012

class Settings {
    public:
        Settings(Config &config, HINSTANCE instance, HWND parent);

        void show(void);
    private:
        static INT_PTR CALLBACK _proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        void _on_init(HWND hwnd);
        void _on_ok(HWND hwnd);
        void _register(void);

        Config &_config;
        HINSTANCE _instance;
        HWND _parent;
};

static Settings *settings = nullptr;
