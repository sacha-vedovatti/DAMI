/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Config header file
*/

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <cstdint>
#include <iostream>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <atomic>

static constexpr const wchar_t *MUTEX_NAME = L"Global\\DAMI_SingleInstance";

typedef struct config_s {
    /* Discord RPC Settings */
    bool show_title = true;
    bool show_artist = true;
    bool show_album = true;
    bool show_cover = true;
    bool show_timestamps = true;

    /* Main Settings */
    bool auto_start = false;
} config_t;

class Config {
    public:
        Config(const std::string &path);
        bool load(void);
        bool save(void);

        config_t *get_settings(void) const;
    private:
        bool _write_save(void);

        /* Path */
        std::string _path;

        /* Config */
        config_t _settings;
};

uint64_t discord_client_id(void);
std::string default_path(void);
std::wstring icon_path(void);
