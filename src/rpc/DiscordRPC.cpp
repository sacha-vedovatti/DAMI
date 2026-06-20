/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Discord SDK Rich Presence
*/

#include "DiscordRPC.hpp"

DiscordRPC::DiscordRPC(uint64_t id) : _id(id) { }

DiscordRPC::~DiscordRPC()
{
    if (_pipe != INVALID_HANDLE_VALUE)
        clear();
    _disconnect();
}

bool DiscordRPC::_connect(void)
{
    for (int i = 0; i < 10; i++) {
        std::string pipe_name = "\\\\.\\pipe\\discord-ipc-" + std::to_string(i);
        _pipe = CreateFileA(pipe_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (_pipe != INVALID_HANDLE_VALUE)
            return true;
    }
    return false;
}

void DiscordRPC::_disconnect(void)
{
    if (_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(_pipe);
        _pipe = INVALID_HANDLE_VALUE;
    }
}

bool DiscordRPC::_reconnect(void)
{
    if (!_connect())
        return false;
    if (!_handshake()) {
        _disconnect();
        return false;
    }
    return true;
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
        _disconnect();
        return false;
    }

    char header[8] = {};
    DWORD bytes = 0;
    if (!ReadFile(_pipe, header, 8, &bytes, nullptr)) {
        _disconnect();
        return false;
    }
    if (bytes == 8) {
        uint32_t response = 0;
        std::memcpy(&response, &header[4], 4);
        if (response > 0 && response < 65536) {
            std::string body(response, '\0');
            if (!ReadFile(_pipe, &body[0], response, &bytes, nullptr)) {
                _disconnect();
                return false;
            }
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
    return _reconnect();
}

void DiscordRPC::tick(void)
{
    if (_pipe != INVALID_HANDLE_VALUE)
        return;
    if (_reconnect() && !_last_payload.empty())
        _send(1, _last_payload);
}

static std::string json_escape(const std::string &string)
{
    std::string result;

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
                } else {
                    result += static_cast<char>(c);
                }
        }
    }
    return result;
}

static std::string ensure_min_length(const std::string &str, size_t min_len = 2)
{
    if (str.length() >= min_len)
        return str;
    return str + std::string(min_len - str.length(), ' ');
}

std::string DiscordRPC::_build_payload(const TrackInfo &track)
{
    std::ostringstream json;
    const std::string &image = track.image.empty() ? DISCORD_APP_ASSET_KEY : track.image;

    _nonce++;
    json << "{"
         <<   "\"cmd\":\"SET_ACTIVITY\"," 
         <<   "\"args\":{" 
         <<     "\"pid\":" << GetCurrentProcessId() << ","
         <<     "\"activity\":{" 
         <<       "\"type\":2,"
         <<       "\"details\":\"" << json_escape(ensure_min_length(track.title)) << "\"," 
         <<       "\"state\":\"" << json_escape(ensure_min_length(track.artist)) << "\",";

    if (track.start > 0) {
        json << "\"timestamps\":{" 
             <<   "\"start\":" << track.start;
        if (track.end > track.start)
            json << ",\"end\":" << track.end;
        json << "},";
    }

    json << "\"assets\":{" 
         <<         "\"large_image\":\"" << json_escape(image) << "\"," 
         <<         "\"large_text\":\"" << json_escape(track.album) << "\""
         <<       "}"
         <<     "}"
         <<   "},"
         <<   "\"nonce\":\"" << _nonce << "\""
         << "}";
    return json.str();
}

void DiscordRPC::update(const TrackInfo &track)
{
    const std::string payload = _build_payload(track);
    _last_payload = payload;

    if (!_send(1, payload))
        _disconnect();
}

void DiscordRPC::clear(void)
{
    std::ostringstream json;

    _nonce++;
    _last_payload.clear();
    json << "{"
         <<   "\"cmd\":\"SET_ACTIVITY\"," 
         <<   "\"args\":{\"pid\":" << GetCurrentProcessId() << ",\"activity\":null},"
         <<   "\"nonce\":\"" << _nonce << "\""
         << "}";
    _send(1, json.str());
}
