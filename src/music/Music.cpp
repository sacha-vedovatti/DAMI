/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Apple Music handler
*/

#include "Music.hpp"

uint64_t load_config(const std::string &configPath)
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

void Music::_extract(void)
{
    std::string parsed_artist, parsed_album;
    size_t sep_pos = std::string::npos;
    const char *separators[] = {" — ", " – "};

    for (const char *sep : separators) {
        sep_pos = _author.find(sep);
        if (sep_pos != std::string::npos) {
            parsed_artist = _author.substr(0, sep_pos);
            parsed_album  = _author.substr(sep_pos + std::strlen(sep));
            _author = parsed_artist;
            _album  = parsed_album;
            break;
        }
    }
}

Music::Music(Server &server) : _server(server) { }

int Music::run(DiscordRPC &rpc)
{
    bool loaded = false;
    bool has_cover = false;

    while (true) {
        loaded = load().get();
        if (!loaded) {
            _old_title.clear();
            _old_author.clear();
            _cover_url.clear();
            rpc.clear();
        } else {
            if (_title != _old_title || _author != _old_author) {
                _old_title = _title;
                _old_author = _author;
                has_cover = _load_cover().get();

                std::cout << "[PLAYING] Now playing:" << std::endl;
                std::cout << "\tTITRE: "  << _title  << std::endl;
                std::cout << "\tARTIST: " << _author << std::endl;
                std::cout << "\tALBUM: "  << _album  << std::endl;
                std::cout << "\tCOVER: "  << (has_cover ? _cover_url : "[NOT_FOUND]") << std::endl;

                rpc.update(_title, _author, _album, has_cover ? _cover_url : "");
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(POLL_INTERVAL_SECONDS));
    }
    return 0;
}

winrt::Windows::Foundation::IAsyncOperation<bool> Music::load(void)
{
    _manager = co_await winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync();
    _session = _manager.GetCurrentSession();
    if (!_session)
        co_return _clear();
    if (!_verify_source())
        co_return _clear();
    _info = co_await _session.TryGetMediaPropertiesAsync();
    if (!_info)
        co_return _clear();
    _title = winrt::to_string(_info.Title());
    _author = winrt::to_string(_info.Artist());
    _album = winrt::to_string(_info.AlbumTitle());
    if (_album.empty())
        _extract();
    co_return !_title.empty() || !_author.empty();
}

std::string Music::_build(const std::vector<uint8_t> &bytes, const std::string &mime, const std::string &boundary)
{
    std::string filename = "cover.jpg";
    if (mime == "image/png")
        filename = "cover.png";
    if (mime == "image/webp")
        filename = "cover.webp";

    std::string body = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"reqtype\"\r\n\r\nfileupload\r\n--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"fileToUpload\"; filename=\"" + filename + "\"\r\nContent-Type: " + mime + "\r\n\r\n";
    body.append(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    body += "\r\n--" + boundary + "--\r\n";
    return body;
}

static std::string close_connection(HINTERNET session, HINTERNET connect, HINTERNET request)
{
    if (session)
        WinHttpCloseHandle(session);
    if (connect)
        WinHttpCloseHandle(connect);
    if (request)
        WinHttpCloseHandle(request);
    return "";
}

std::string Music::_send_to_catbox(const std::string &body, const std::string &boundary)
{
    const std::string content_type = "multipart/form-data; boundary=" + boundary;
    HINTERNET session = WinHttpOpen(L"DAMI/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session)
        return "";

    HINTERNET connect = WinHttpConnect(session, L"catbox.moe", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connect)
        return close_connection(session, nullptr, nullptr);

    HINTERNET request = WinHttpOpenRequest(connect, L"POST", L"/user/api.php", nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request)
        return close_connection(session, connect, nullptr);

    std::string header = "Content-Type: " + content_type;
    std::wstring wheader(header.begin(), header.end());
    WinHttpAddRequestHeaders(request, wheader.c_str(), (DWORD) -1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);

    BOOL sent = WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID) body.data(), (DWORD) body.size(), (DWORD) body.size(), 0);
    if (!sent)
        return close_connection(session, connect, request);
    WinHttpReceiveResponse(request, nullptr);

    std::string response = "";
    DWORD available = 0;
    while (WinHttpQueryDataAvailable(request, &available) && available > 0) {
        std::string chunk(available, '\0');
        DWORD read = 0;
        WinHttpReadData(request, &chunk[0], available, &read);
        chunk.resize(read);
        response += chunk;
    }
    close_connection(session, connect, request);
    while (!response.empty() && (response.back() == '\n' || response.back() == '\r' || response.back() == ' '))
        response.pop_back();
    return response;
}

bool Music::_validate(const std::string &response)
{
    if (response.rfind("https://files.catbox.moe/", 0) != 0) {
        std::cerr << "[COVER] catbox.moe unexpected response: " << response << std::endl;
        return false;
    }
    return true;
}

std::string Music::_upload(const std::vector<uint8_t> &bytes, const std::string &mime)
{
    std::string boundary = "----DAMIBoundary7MA4YWxkTrZu0gW";
    std::string body = _build(bytes, mime, boundary);
    std::string response = _send_to_catbox(body, boundary);

    if (!_validate(response))
        return "";
    return response;
}

winrt::Windows::Foundation::IAsyncOperation<bool> Music::_load_cover(void)
{
    if (!_info)
        co_return false;

    auto thumbnail = _info.Thumbnail();
    if (!thumbnail)
        co_return false;

    winrt::Windows::Storage::Streams::IRandomAccessStream stream{nullptr};
    try {
        stream = co_await thumbnail.OpenReadAsync();
    } catch (...) {
        co_return false;
    }
    if (!stream)
        co_return false;

    uint64_t size = stream.Size();
    if (size == 0 || size > 10 * 1024 * 1024)
        co_return false;

    std::string mime = "image/jpeg";

    auto input = stream.GetInputStreamAt(0);
    winrt::Windows::Storage::Streams::DataReader reader(input);
    try {
        co_await reader.LoadAsync(static_cast<uint32_t>(size));
    } catch (...) {
        co_return false;
    }

    std::vector<uint8_t> bytes(static_cast<size_t>(size));
    reader.ReadBytes(bytes);
    _server.set_cover(bytes, mime);

    std::string url = _upload(bytes, mime);
    if (url.empty())
        co_return false;

    _cover_url = url;
    co_return true;
}

std::string Music::getTitle(void) const
{
    return _title;
}

void Music::setTitle(std::string &title)
{
    _title = title;
}

std::string Music::getAuthor(void) const
{
    return _author;
}

void Music::setAuthor(std::string &author)
{
    _author = author;
}

std::string Music::getSource(void) const
{
    return _source;
}

bool Music::hasTrack(void) const
{
    return !_title.empty() || !_author.empty();
}

bool Music::_clear(void)
{
    _title.clear();
    _author.clear();
    _source.clear();
    _album.clear();
    return false;
}

bool Music::_verify_source(void)
{
    auto app_id = _session.SourceAppUserModelId();

    _source = winrt::to_string(app_id);
    return app_id.starts_with(APPLE_MUSIC_APP_ID);
}
