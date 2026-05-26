/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Apple Music handler
*/

#include "Music.hpp"

void Music::_exctract(void)
{
    std::string parsed_artist, parsed_album;
    size_t sep_pos = std::string::npos;
    const char *separators[] = {" - ", " — ", " – ", " | ", " / "};

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

Music::Music(Server &server) : _server {}

int Music::run(DiscordRPC &rpc)
{
    bool loaded = false;
    bool has_cover = false;

    while (true) {
        loaded = load().get();
        if (!loaded) {
            _old_title.clear();
            _old_author.clear();
            rpc.clear();
        } else {
            if (_title != _old_title || _author != _old_author) {
                _old_title = _title;
                _old_author = _author;
                has_cover = _load_cover().get();

                std::cout << "[PLAYING] Now playing:" << std::endl;
                std::cout << "\tTITRE: " << _title << std::endl;
                std::cout << "\tARTIST: " << _author << std::endl;
                std::cout << "\tALBUM: " << _album << std::endl;
                std::cout << "\tCOVER: " << (has_cover ? Server::url() : "[NOT_FOUND]") << std::endl;

                rpc.update(_title, _author, has_cover ? Server::url() : "");
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
    return false;
}

bool Music::_verify_source(void)
{
    auto app_id = _session.SourceAppUserModelId();

    _source = winrt::to_string(app_id);
    if (!app_id.starts_with(APPLE_MUSIC_APP_ID))
        return false;
    return true;
}
