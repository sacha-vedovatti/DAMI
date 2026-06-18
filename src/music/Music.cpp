/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Apple Music handler
*/

#include "Music.hpp"

Music::Music(Server &server, const config_t &conf) : _server(server), _config(conf) { }

bool Music::init(void)
{
    auto op = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync();

    _manager = op.get();
    return _manager != nullptr;
}

void Music::clear_cache(void)
{
    _cover_url.clear();
    _has_cover = false;
    _server.set_cover({}, "");
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

void Music::_print(bool has_image)
{
    std::cout << "[PLAYING] Now playing:" << std::endl;
    std::cout << "\tTITRE: "  << _title  << std::endl;
    std::cout << "\tARTIST: " << _author << std::endl;
    std::cout << "\tALBUM: "  << _album  << std::endl;
    std::cout << "\tCOVER: "  << (has_image ? _cover_url : "[NOT_FOUND]") << std::endl;
}

void Music::update(DiscordRPC &rpc)
{
    bool track_changed = (_title != _old_title || _author != _old_author);
    bool state_changed = (_is_playing != _tmp_playing);

    if (track_changed) {
        _old_title  = _title;
        _old_author = _author;
        if (_config.show_cover)
            _has_cover = _load_cover().get();
        else
            _has_cover = false;
        _print(_has_cover);
    }
    if (track_changed || state_changed) {
        _tmp_playing = _is_playing;
        TrackInfo info {
            _config.show_title  ? _title  : "",
            _config.show_artist ? _author : "",
            _config.show_album  ? _album  : "",
            (_config.show_cover && _has_cover) ? _cover_url : "",
        };
        if (_config.show_timestamps) {
            info.start = _start;
            info.end   = _end;
        }
        rpc.update(info);
    }
}

winrt::Windows::Foundation::IAsyncOperation<bool> Music::load(void)
{
    if (!_manager)
        co_return _clear();
    if (!_select_session())
        co_return _clear();
    try {
        _info = co_await _session.TryGetMediaPropertiesAsync();
    } catch (...) {
        co_return _clear();
    }
    if (!_info)
        co_return _clear();
    _title = winrt::to_string(_info.Title());
    _author = winrt::to_string(_info.Artist());
    _album = winrt::to_string(_info.AlbumTitle());
    if (_album.empty())
        _extract();
    _get_timestamp();
    co_return !_title.empty() || !_author.empty();
}

void Music::_get_timestamp(void)
{
    auto playback = _session.GetPlaybackInfo();
    _is_playing = (playback && playback.PlaybackStatus() == winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing);
    _start = 0;
    _end = 0;
    if (!_is_playing)
        return;

    auto timeline = _session.GetTimelineProperties();
    if (timeline) {
        using namespace std::chrono;
        int64_t pos = duration_cast<seconds>(timeline.Position()).count();
        int64_t end_time = duration_cast<seconds>(timeline.EndTime()).count();
        int64_t time = static_cast<int64_t>(std::time(nullptr));

        _start = time - pos;
        if (end_time > pos)
            _end = _start + end_time;
    }
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

static bool is_valid_catbox_url(const std::string &response)
{
    static const std::string prefix = "https://";
    static const std::string host = "files.catbox.moe";
    if (response.rfind(prefix, 0) != 0)
        return false;

    const size_t host_begin = prefix.size();
    const size_t host_end = response.find('/', host_begin);
    if (host_end == std::string::npos || response.compare(host_begin, host_end - host_begin, host) != 0)
        return false;

    const size_t path_begin = host_end;
    if (path_begin >= response.size() || response[path_begin] != '/')
        return false;
    if (response.find_first_of("?#", path_begin) != std::string::npos)
        return false;
    if (response.find('/', path_begin + 1) != std::string::npos)
        return false;
    for (size_t i = path_begin + 1; i < response.size(); i++) {
        unsigned char c = static_cast<unsigned char>(response[i]);
        bool allowed = std::isalnum(c) || c == '.' || c == '_' || c == '-';
        if (!allowed)
            return false;
    }
    return (response.size() > path_begin + 1);
}

bool Music::_validate(const std::string &response)
{
    if (!is_valid_catbox_url(response)) {
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

static std::string detect_image_mime(const std::vector<uint8_t> &bytes)
{
    if (bytes.size() >= 8 && bytes[0] == 0x89 && bytes[1] == 0x50 && bytes[2] == 0x4E && bytes[3] == 0x47 && bytes[4] == 0x0D && bytes[5] == 0x0A && bytes[6] == 0x1A && bytes[7] == 0x0A)
        return "image/png";
    if (bytes.size() >= 3 && bytes[0] == 0xFF && bytes[1] == 0xD8 && bytes[2] == 0xFF)
        return "image/jpeg";
    if (bytes.size() >= 12 && bytes[0] == 'R' && bytes[1] == 'I' && bytes[2] == 'F' && bytes[3] == 'F' && bytes[8] == 'W' && bytes[9] == 'E' && bytes[10] == 'B' && bytes[11] == 'P')
        return "image/webp";
    return "";
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

    auto input = stream.GetInputStreamAt(0);
    winrt::Windows::Storage::Streams::DataReader reader(input);
    try {
        co_await reader.LoadAsync(static_cast<uint32_t>(size));
    } catch (...) {
        co_return false;
    }

    std::vector<uint8_t> bytes(static_cast<size_t>(size));
    reader.ReadBytes(bytes);

    std::string mime = detect_image_mime(bytes);
    if (mime.empty())
        co_return false;
    _server.set_cover(bytes, mime);

    std::string url = _upload(bytes, mime);
    if (url.empty())
        co_return false;
    _cover_url = url;
    co_return true;
}



bool Music::_clear(void)
{
    _title.clear();
    _author.clear();
    _album.clear();
    return false;
}

bool Music::_verify_source(void)
{
    auto app_id = _session.SourceAppUserModelId();
    return app_id.starts_with(APPLE_MUSIC_APP_ID);
}

bool Music::_is_apple_music_session(const winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession &session)
{
    if (!session)
        return false;
    return session.SourceAppUserModelId().starts_with(APPLE_MUSIC_APP_ID);
}

bool Music::_select_session(void)
{
    auto current = _manager.GetCurrentSession();
    if (_is_apple_music_session(current)) {
        _session = current;
        return _verify_source();
    }

    auto sessions = _manager.GetSessions();
    winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession fallback{nullptr};

    for (uint32_t i = 0; i < sessions.Size(); i++) {
        auto session = sessions.GetAt(i);
        if (!_is_apple_music_session(session))
            continue;

        if (!fallback)
            fallback = session;

        auto playback = session.GetPlaybackInfo();
        if (playback && playback.PlaybackStatus() == winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing) {
            _session = session;
            return _verify_source();
        }
    }

    if (fallback) {
        _session = fallback;
        return _verify_source();
    }
    return false;
}
