/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Apple Music handler
*/

#include "music.hpp"

static std::string extract_artist(const std::string &author)
{
    const char *separators[] = {" - ", " — ", " – ", " | ", " / "};

    for (const char *separator : separators) {
        size_t pos = author.find(separator);
        if (pos != std::string::npos)
            return author.substr(0, pos);
    }
    return author;
}

static bool split_artist_album(const std::string &author, std::string &artist, std::string &album)
{
    const char *separators[] = {" - ", " — ", " – ", " | ", " / "};

    for (const char *separator : separators) {
        size_t pos = author.find(separator);
        if (pos != std::string::npos) {
            artist = author.substr(0, pos);
            album = author.substr(pos + std::strlen(separator));
            return true;
        }
    }
    return false;
}

int Music::run(DiscordRPC &rpc)
{
    bool loaded = false;

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
                _img = Cover::fetch(extract_artist(_author), _title, _album);

                std::cout << "[PLAYING] Now playing:" << std::endl;
                std::cout << "\tTITRE: " << _title << std::endl;
                std::cout << "\tARTIST: " << _author << std::endl;
                std::cout << "\tALBUM: " << _album << std::endl;
                std::cout << "\tCOVER: " << (_img.empty() ? "introuvable" : _img) << std::endl;

                rpc.update(_title, _author, _img);
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
    if (_album.empty()) {
        std::string parsed_artist = _author;

        if (split_artist_album(_author, parsed_artist, _album))
            _author = parsed_artist;
    }
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
