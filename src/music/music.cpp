/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Apple Music handler
*/

#include "music.hpp"

int Music::run(void)
{
    while (true) {
        bool loaded = LoadAsync().get();

        if (!loaded) {
            _old_title.clear();
            _old_author.clear();
        } else {
            if (_title != _old_title || _author != _old_author) {
                _old_title = _title;
                _old_author = _author;
                std::cout << "[PLAYING] " << _title << std::endl;
                std::cout << "\t\t" << _author << "\n" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(POLL_INTERVAL_SECONDS));
    }
    return 0;
}

winrt::Windows::Foundation::IAsyncOperation<bool> Music::LoadAsync(void)
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
