/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Discord SDK Rich Presence
*/

#pragma once

#include <string>
#include <iostream>
#include <discord.h>

class DiscordRPC {
    public:
        DiscordRPC(discord::ClientId id);
        ~DiscordRPC();

        bool init(void);
        void update(const std::string &title, const std::string &artist, const std::string &img_url);
        void clear(void);
        void callbacks(void);

    private:
        discord::ClientId _id;
        discord::Core *_core{nullptr};
};
