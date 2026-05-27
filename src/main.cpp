/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** main file
*/

#include "music/Music.hpp"

int main()
{
    winrt::init_apartment();

    uint64_t client_id = load_config();
    if (client_id == 0)
        return 84;

    Server server;
    if (!server.start())
        return 84;

    DiscordRPC rpc(client_id);
    if (!rpc.init())
        return 84;

    Music music(server);
    return music.run(rpc);
}
