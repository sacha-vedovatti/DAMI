/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Apple Music handler
*/

#pragma once

#include "../server/Server.hpp"
#include "../rpc/DiscordRPC.hpp"

#include <string>
#include <vector>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/base.h>
#include <iostream>
#include <winhttp.h>
#include <fstream>
#include <sstream>
#include <iomanip>

static constexpr std::wstring_view APPLE_MUSIC_APP_ID = L"AppleInc.AppleMusic";
static constexpr int POLL_INTERVAL_SECONDS = 5;

class Music {
    public:
        explicit Music(Server &server);

        int run(DiscordRPC &rpc);
        winrt::Windows::Foundation::IAsyncOperation<bool> load(void);

        bool hasTrack(void) const;

        std::string getTitle(void) const;
        void setTitle(std::string &title);

        std::string getAuthor(void) const;
        void setAuthor(std::string &author);

        std::string getSource(void) const;
    private:
        /* METHODS */

        void _print(bool has_image);
        bool _clear(void);
        void _update(DiscordRPC &rpc, bool &has_cover, bool &old_playing);
        bool _verify_source(void);
        void _get_timestamp(void);
        void _extract(void);
        bool _validate(const std::string &response);
        std::string _upload(const std::vector<uint8_t> &bytes, const std::string &mime);
        std::string _build(const std::vector<uint8_t> &bytes, const std::string &mime, const std::string &boundary);
        std::string _send_to_catbox(const std::string &body, const std::string &boundary);
        winrt::Windows::Foundation::IAsyncOperation<bool> _load_cover(void);

        /* ATTRIBUTES */

        Server &_server;

        winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager _manager{nullptr};
        winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession _session{nullptr};
        winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionMediaProperties _info{nullptr};

        std::string _source;
        std::string _title;
        std::string _author;
        std::string _album;
        std::string _cover_url;
        std::string _track_url;

        std::string _old_title;
        std::string _old_author;

        bool _is_playing = false;
        int64_t _start = 0;
        int64_t _end = 0;
};
