/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** HTTP Server
*/

#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

class Server {
    public:
        Server();
        ~Server();

        bool start(void);
        void set_cover(std::vector<uint8_t> bytes, const std::string &mime);

        static std::string url(void);

    private:
        void _serve(void);
        void _handle(SOCKET client);
        bool _error(void);

        uint16_t _PORT = 49152;
        std::thread _thread;
        std::atomic<bool> _running{false};
        SOCKET _socket{INVALID_SOCKET};

        std::mutex _mutex;
        std::vector<uint8_t> _bytes;
        std::string _mime{"image/jpeg"};
};
