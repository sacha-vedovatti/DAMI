/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Discord SDK Rich Presence
*/

#include "discord.rpc.hpp"

namespace {
static constexpr const char *DISCORD_APP_ASSET_KEY = "apple_music";

static bool looks_like_url(const std::string &value)
{
    return value.rfind("http://", 0) == 0 || value.rfind("https://", 0) == 0;
}
}

DiscordRPC::DiscordRPC(discord::ClientId id) : _id(id) { }

DiscordRPC::~DiscordRPC()
{
    if (_core) {
        clear();
        delete _core;
    }
}

bool DiscordRPC::init(void)
{
    auto result = discord::Core::Create(_id, DiscordCreateFlags_NoRequireDiscord, &_core);

    if (result != discord::Result::Ok) {
        std::cerr << "[DISCORD] RPC initialization failed: " << (int) result << std::endl;
        return false;
    }
    std::cout << "[DISCORD] SDK initialized." << std::endl;
    return true;
}

void DiscordRPC::update(const std::string &title, const std::string &artist, const std::string &img_url)
{
    discord::Activity activity{};
    const char *large_image = DISCORD_APP_ASSET_KEY;

    if (!_core)
        return;
    if (!img_url.empty() && !looks_like_url(img_url))
        large_image = img_url.c_str();
    activity.SetType(discord::ActivityType::Listening);
    activity.SetDetails(title.c_str());
    activity.SetState(artist.c_str());
    activity.GetAssets().SetLargeImage(large_image);
    activity.GetAssets().SetLargeText(title.c_str());
    activity.GetAssets().SetSmallImage(DISCORD_APP_ASSET_KEY);
    activity.GetAssets().SetSmallText("Apple Music");
    _core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result != discord::Result::Ok)
            std::cerr << "[DISCORD] Activity update failed: " << (int) result << std::endl;
    });
}

void DiscordRPC::clear(void)
{
    if (!_core)
        return;
    _core->ActivityManager().ClearActivity([](discord::Result) {});
}

void DiscordRPC::callbacks(void)
{
    if (_core)
        _core->RunCallbacks();
}
