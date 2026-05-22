/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Discord SDK Rich Presence
*/

#include "discord.rpc.hpp"

DiscordRPC::DiscordRPC(uint64_t id) : _id(id) { }

DiscordRPC::~DiscordRPC()
{
    if (_pipe != INVALID_HANDLE_VALUE) {
        clear();
        CloseHandle(_pipe);
        _pipe = INVALID_HANDLE_VALUE;
    }
}

bool DiscordRPC::_connect(void)
{
    for (int i = 0; i < 10; i++) {
        std::string pipe_name = "\\\\.\\pipe\\discord-ipc-" + std::to_string(i);
        _pipe = CreateFileA(pipe_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (_pipe != INVALID_HANDLE_VALUE)
            return true;
    }
    std::cerr << "[DISCORD] Could not connect to Discord pipe." << std::endl;
    return false;
}

bool DiscordRPC::_send(int opcode, const std::string &payload)
{
    if (_pipe == INVALID_HANDLE_VALUE)
        return false;
    
    uint32_t op = static_cast<uint32_t>(opcode);
    uint32_t len = static_cast<uint32_t>(payload.size());
    std::string frame(8 + len, '\0');
    std::memcpy(&frame[0], &op, 4);
    std::memcpy(&frame[4], &len, 4);
    std::memcpy(&frame[8], payload.data(), len);

    DWORD written = 0;
    if (!WriteFile(_pipe, frame.data(), static_cast<DWORD>(frame.size()), &written, nullptr)) {
        std::cerr << "[DISCORD] WriteFile failed: " << GetLastError() << std::endl;
        return false;
    }

    char header[8] = {};
    DWORD bytes = 0;
    ReadFile(_pipe, header, 8, &bytes, nullptr);
    if (bytes == 8) {
        uint32_t response = 0;
        std::memcpy(&response, &header[4], 4);
        if (response > 0 && response < 65536) {
            std::string body(response, '\0');
            ReadFile(_pipe, &body[0], response, &bytes, nullptr);
        }
    }
    return true;
}

bool DiscordRPC::_handshake(void)
{
    std::ostringstream handshake;

    handshake << "{\"v\":1,\"client_id\":\"" << _id << "\"}";
    return _send(0, handshake.str());
}

bool DiscordRPC::init(void)
{
    if (!_connect())
        return false;
    if (!_handshake())
        return false;
    std::cout << "[DISCORD] IPC connected." << std::endl;
    return true;
}

static std::string json_escape(const std::string &string)
{
    std::string result = "";

    result.reserve(string.size());
    for (unsigned char c : string) {
        switch (c) {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (c < 0x20) {
                    char buffer[8];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                    result += buffer;
                } else
                    result += static_cast<char>(c);
        }
    }
    return result;
}

std::string DiscordRPC::_build_payload(const std::string &title, const std::string &artist, const std::string &img_url)
{
    std::ostringstream json;

    _nonce++;
    json << "{"
       <<   "\"cmd\":\"SET_ACTIVITY\","
       <<   "\"args\":{"
       <<     "\"pid\":" << GetCurrentProcessId() << ","
       <<     "\"activity\":{"
       <<       "\"type\":2,"
       <<       "\"details\":\"" << json_escape(title)  << "\","
       <<       "\"state\":\""   << json_escape(artist) << "\","
       <<       "\"assets\":{"
       <<         "\"large_image\":\"" << json_escape(img_url) << "\","
       <<         "\"large_text\":\""  << json_escape(title)   << "\","
       <<         "\"small_image\":\"apple_music\","
       <<         "\"small_text\":\"Apple Music\""
       <<       "}"
       <<     "}"
       <<   "},"
       <<   "\"nonce\":\"" << _nonce << "\""
       << "}";
    return json.str();
}

void DiscordRPC::update(const std::string &title, const std::string &artist, const std::string &img_url)
{
    const std::string &image = img_url.empty() ? "apple_music" : img_url;
    const std::string payload = _build_payload(title, artist, image);

    if (!_send(1, payload)) {
        std::cerr << "[DISCORD] Send failed, attempting reconnect..." << std::endl;
        if (_pipe != INVALID_HANDLE_VALUE) {
            CloseHandle(_pipe);
            _pipe = INVALID_HANDLE_VALUE;
        }
        if (_connect() && _handshake())
            _send(1, payload);
    }
}

void DiscordRPC::clear(void)
{
    std::ostringstream json;

    _nonce++;
    json << "{"
         <<   "\"cmd\":\"SET_ACTIVITY\","
         <<   "\"args\":{\"pid\":" << GetCurrentProcessId() << ",\"activity\":null},"
         <<   "\"nonce\":\"" << _nonce << "\""
         << "}";
    _send(1, json.str());
}
