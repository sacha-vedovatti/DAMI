/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Discord SDK Rich Presence
*/

#pragma once

#include <string>
#include <iostream>
#include <cstdint>
#include <cctype>
#include <cstdio>
#include <windows.h>
#include <cstring>
#include <sstream>

static constexpr const char *DISCORD_APP_ASSET_KEY = "apple_music";

class DiscordRPC {
    public:
        DiscordRPC(uint64_t id);
        ~DiscordRPC();

        bool init(void);
        void update(const std::string &title, const std::string &artist, const std::string &album, const std::string &img_url);
        void clear(void);
        // void callbacks(void);

    private:
        bool _connect(void);
        bool _send(int opcode, const std::string &payload);
        bool _handshake(void);
        std::string _build_payload(const std::string &title, const std::string &artist, const std::string &album, const std::string &img_url);

        uint64_t _id;
        HANDLE _pipe{INVALID_HANDLE_VALUE};
        int _nonce{0};
};
