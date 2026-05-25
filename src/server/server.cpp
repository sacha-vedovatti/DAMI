/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** HTTP Server
*/

#include "server.hpp"

Server::Server()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}

Server::~Server()
{
    _running = false;
    if (_socket != INVALID_SOCKET) {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
    }
    if (_thread.joinable())
        _thread.join();
    WSACleanup();
}

bool Server::_error(void)
{
    std::cerr << "[SERVER] Error: " << WSAGetLastError() << std::endl;
    if (_socket != INVALID_SOCKET) {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
    }
    return false;
}

bool Server::start(void)
{
    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket == INVALID_SOCKET)
        return _error();

    int opt = 1;
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &opt, sizeof(opt));

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(_PORT);
    if (bind(_socket, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
        return _error();
    if (listen(_socket, SOMAXCONN) == SOCKET_ERROR)
        return _error();
    _running = true;
    _thread = std::thread(&Server::_serve, this);
    std::cout << "[SERVER] Listening on " << url() << std::endl;
    return true;
}

void Server::set_cover(std::vector<uint8_t> bytes, const std::string &mime)
{
    std::lock_guard<std::mutex> lock(_mutex);

    _bytes = std::move(bytes);
    _mime = mime;
}
