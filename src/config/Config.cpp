/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Configuration functions
*/

#include "Config.hpp"

Config::Config(const std::string &path) : _path(path) { }

/*    -  LOAD CONFIG  -    */

static bool parse_bool(const std::string &json, const std::string &key, bool fallback = true)
{
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);

    if (pos == std::string::npos)
        return fallback;
    pos = json.find(':', pos);
    if (pos == std::string::npos)
        return fallback;
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t'))
        pos++;
    if (json.compare(pos, 4, "true") == 0) 
        return true;
    if (json.compare(pos, 5, "false") == 0)
        return false;
    return fallback;
}

bool Config::load(void)
{
    std::ifstream file(_path);
    if (!file.is_open()) {
        std::cerr << "[CONFIG] Cannot open config file: " << _path << " (creating default config)" << std::endl;
        return _write_save();
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    const std::string json = ss.str();
    _settings.show_title = parse_bool(json, "show_title", true);
    _settings.show_artist = parse_bool(json, "show_artist", true);
    _settings.show_album = parse_bool(json, "show_album", true);
    _settings.show_cover = parse_bool(json, "show_cover", true);
    _settings.show_timestamps = parse_bool(json, "show_timestamps", true);
    _settings.auto_start = parse_bool(json, "auto_start", false);
    return true;
}

/*    -  SAVE CONFIG  -    */

bool Config::save(void)
{
    if (!_write_save())
        return false;

    static constexpr const wchar_t *RUN_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    static constexpr const wchar_t *VALUE_NAME = L"DAMI";
    HKEY h_key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_SET_VALUE, &h_key) == ERROR_SUCCESS) {
        if (_settings.auto_start) {
            wchar_t exe_path[MAX_PATH] = {};
            GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
            RegSetValueExW(h_key, VALUE_NAME, 0, REG_SZ, reinterpret_cast<const BYTE *>(exe_path), (DWORD)((wcslen(exe_path) + 1) * sizeof(wchar_t)));
        } else
            RegDeleteValueW(h_key, VALUE_NAME);
        RegCloseKey(h_key);
    }
    return true;
}

bool Config::_write_save()
{
    std::ofstream file(_path);
    if (!file.is_open()) {
        std::cerr << "[CONFIG] Cannot write config file: " << _path << std::endl;
        return false;
    }

    auto bool_to_str = [](bool value) -> const char * {
        return value ? "true" : "false";
    };
    file << "{\n"
        << "    \"show_title\": " << bool_to_str(_settings.show_title) << ",\n"
        << "    \"show_artist\": " << bool_to_str(_settings.show_artist) << ",\n"
        << "    \"show_album\": " << bool_to_str(_settings.show_album) << ",\n"
        << "    \"show_cover\": " << bool_to_str(_settings.show_cover) << ",\n"
        << "    \"show_timestamps\": " << bool_to_str(_settings.show_timestamps) << ",\n"
        << "    \"auto_start\": " << bool_to_str(_settings.auto_start) << "\n"
        << "}\n";
    return true;
}

config_t *Config::get_settings(void) const
{
    return const_cast<config_t *>(&_settings);
}

/* DEFAULT PATH */
std::string default_path(void)
{
    char buffer[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);

    std::string path(buffer);
    size_t slash = path.find_last_of("\\/");
    if (slash != std::string::npos)
        path = path.substr(0, slash + 1);
    return path + "config.json";
}

std::wstring icon_path(void)
{
    wchar_t buffer[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);

    std::wstring path(buffer);
    size_t slash = path.find_last_of(L"\\/");
    if (slash != std::wstring::npos)
        path = path.substr(0, slash + 1);
    return path + L"DAMI.ico";
}

/* DISCORD CLIENT ID */
uint64_t discord_client_id(void)
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
