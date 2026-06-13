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
#include <vector>

static constexpr const char *DISCORD_APP_ASSET_KEY = "apple_music";

struct TrackInfo {
    std::string title;
    std::string artist;
    std::string album;
    std::string image;
    int64_t start = 0;
    int64_t end = 0;
};

class DiscordRPC {
    public:
        DiscordRPC(uint64_t id);
        ~DiscordRPC();

        bool init(void);
        void tick(void);
        void update(const TrackInfo &track);
        void clear(void);

    private:
        bool _connect(void);
        void _disconnect(void);
        bool _reconnect(void);
        bool _send(int opcode, const std::string &payload);
        bool _handshake(void);
        std::string _build_payload(const TrackInfo &track);

        uint64_t _id;
        HANDLE _pipe{INVALID_HANDLE_VALUE};
        int _nonce{0};
        std::string _last_payload;
};
