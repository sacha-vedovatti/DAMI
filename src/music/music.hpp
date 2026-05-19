/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Apple Music handler
*/

#pragma once

#include <string>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/base.h>
#include <iostream>

static constexpr std::wstring_view APPLE_MUSIC_APP_ID = L"AppleInc.AppleMusic";
static constexpr int POLL_INTERVAL_SECONDS = 5;

class Music {
    public:
        int run(void);
        winrt::Windows::Foundation::IAsyncOperation<bool> LoadAsync(void);
        bool hasTrack(void) const;

        std::string getTitle(void) const;
        void setTitle(std::string &title);

        std::string getAuthor(void) const;
        void setAuthor(std::string &author);

        std::string getSource(void) const;
    private:
        bool _clear(void);
        bool _verify_source(void);

        winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager _manager{nullptr};
        winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession _session{nullptr};
        winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionMediaProperties _info{nullptr};

        std::string _source;

        std::string _title;
        std::string _author;

        std::string _old_title;
        std::string _old_author;
};
