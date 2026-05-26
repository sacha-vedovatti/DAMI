/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** HTTP Server
*/

#include "Server.hpp"

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

std::string Server::url(void)
{
    return "http://127.0.0.1:" + std::to_string(_PORT) + "/cover";
}

void Server::_run(void)
{
    while (_running) {
        SOCKET client = accept(_socket, nullptr, nullptr);

        if (client == INVALID_SOCKET)
            break;
        std::thread([this, client]() {
            _handle(client);
            closesocket(client);
        }).detach();
    }
}

static void send_error(SOCKET client, const char *message)
{
    std::cerr << "[SERVER] [LOG] Error sent at id '" << client << "': " << message << std::endl;
    send(client, message, (int) strlen(message), 0);
    return;
}

void Server::_handle(SOCKET client)
{
    char buffer[2048];
    int received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0)
        return;
    buffer[received] = '\0';

    std::string request(buffer);
    bool has_cover = (req.find("GET /cover") != std::string::npos);
    if (!has_cover)
        return send_error(client, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";)

    std::vector<uint8_t> data;
    std::string mime;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        data = _bytes;
        mime = _mime;
    }
    if (data.empty())
        return send_error(client, "HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n");

    std::string header = "HTTP/1.1 200 OK\r\nContent-Type: " + mime + "\r\nContent-Length: " + std::to_string(data.size()) + "\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n";
    send(client, header.data(), (int) header.size(), 0);
    send(client, (const char *) data.data(), (int) data.size(), 0);
}
