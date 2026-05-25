/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** main file
*/

#include "music/music.hpp"

int main()
{
    winrt::init_apartment();

    Server server;
    if (!server.start())
        return 84;

    DiscordRPC rpc(DISCORD_CLIENT_ID);
    if (!rpc.init())
        return 84;

    Music music(server);
    return music.run(rpc);
}
