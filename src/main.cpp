/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** main file
*/

#include "music/Music.hpp"

uint64_t load_config(void)
{
    #ifdef COMPILED_DISCORD_CLIENT_ID
        return COMPILED_DISCORD_CLIENT_ID;
    #endif

    const char* env_client_id = std::getenv("DISCORD_CLIENT_ID");
    if (env_client_id) {
        try {
            return std::stoull(env_client_id);
        } catch (...) {
            std::cerr << "[CONFIG] Error: Invalid DISCORD_CLIENT_ID env var" << std::endl;
            return 0;
        }
    }
    std::cerr << "[CONFIG] Error: DISCORD_CLIENT_ID not provided (env / compiled) and config file missing/invalid" << std::endl;
    return 0;
}

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
