/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** main file
*/

#include "music/music.hpp"

int main()
{
    winrt::init_apartment();
    Music music;

    return music.run();;
}
