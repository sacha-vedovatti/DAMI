/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Cover fetcher (on iTunes)
*/

#include "cover.hpp"

static std::wstring encode_url(const std::string &s)
{
    std::wstring out;

    for (unsigned char c : s) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            out += (wchar_t) c;
        else if (c == ' ')
            out += L'+';
        else {
            wchar_t buffer[8];
            swprintf(buffer, 8, L"%%%02X", c);
            out += buffer;
        }
    }
    return out;
}

std::string Cover::fetch(const std::string &artist, const std::string &title)
{
    std::wstring query = encode_url(artist + " " + title);
    std::wstring path = L"/search?term=" + query + L"&entity=song&limit=1&country=fr";

    std::string json = _get(L"itunes.apple.com", path);
    if (json.empty())
        return "";

    std::string url = _parse_url(json);
    if (url.empty())
        return "";

    size_t pos = url.find("100x100");
    if (pos != std::string::npos)
        url.replace(pos, 7, "600x600");
    return url;
}

std::string Cover::_get(const std::wstring &host, const std::wstring &path)
{
    std::string result = "";
    HINTERNET session = WinHttpOpen(L"DAMI/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session)
        return "";

    HINTERNET connect = WinHttpConnect(session, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connect) {
        WinHttpCloseHandle(session);
        return "";
    }

    HINTERNET request = WinHttpOpenRequest(connect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request) {
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);
        return "";
    }
    if (!WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);
        return "";
    }
    WinHttpReceiveResponse(request, NULL);

    DWORD bytes = 0;
    char buffer[4096];
    while (WinHttpReadData(request, buffer, sizeof(buffer) - 1, &bytes) && bytes > 0) {
        buffer[bytes] = '\0';
        result += buffer;
    }
    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return result;
}

std::string Cover::_parse_url(const std::string &json)
{
    const std::string key = "\"artworkUrl100\":\"";
    size_t pos = json.find(key);
    if (pos == std::string::npos)
        return "";
    pos += key.size();

    size_t end = json.find('"', pos);
    if (end == std::string::npos)
        return "";
    return json.substr(pos, end - pos);
}
