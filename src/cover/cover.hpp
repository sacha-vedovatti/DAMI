/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Cover fetcher (on iTunes)
*/

#pragma once

#include <string>
#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <sstream>

class Cover {
    public:
        static std::string fetch(const std::string &artist, const std::string &title);

    private:
        // static std::string _build_query(const std::string &artist, const std::string &title);
        static std::string _get(const std::wstring &host, const std::wstring &path);
        static std::string _parse_url(const std::string &json);
};
