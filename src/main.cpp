/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** main file
*/

#include "config/Config.hpp"
#include "tray/TraySys.hpp"
#include "music/Music.hpp"

static std::atomic<bool> running{true};

static void music_thread(config_t *config)
{
    winrt::init_apartment();
    uint64_t client_id = discord_client_id();
    if (client_id == 0)
        return;

    Server server;
    if (!server.start())
        return;

    DiscordRPC rpc(client_id);
    if (!rpc.init())
        return;

    Music music(server, *config);
    music.init();
    while (running.load()) {
        rpc.tick();
        bool loaded = music.load().get();
        if (!loaded) {
            music.clear_cache();
            rpc.clear();
        } else
            music.update(rpc);
        std::this_thread::sleep_for(std::chrono::seconds(POLL_INTERVAL_SECONDS));
    }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    HANDLE mutex = CreateMutexW(nullptr, TRUE, MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(nullptr, L"DAMI is already running.", L"DAMI", MB_OK | MB_ICONINFORMATION);
        return 84;
    }

    Config conf(default_path());
    if (!conf.load())
        return 84;

    std::thread music(music_thread, conf.get_settings());
    TraySys tray(conf, instance);
    if (!tray.init()) {
        running = false;
        music.join();
        ReleaseMutex(mutex);
        CloseHandle(mutex);
        return 84;
    }
 
    int ret = tray.run();
    running = false;
    if (music.joinable())
        music.join();
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    return ret;
}
