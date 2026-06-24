/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Graphical User Interface header file
*/

#pragma once

#include "config/Config.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <thread>
#include <atomic>
#include <mutex>

class GUI {
    public:
        GUI(Config &config);
        ~GUI();

        bool init(void);

        void show(void);
        void hide(void);
        void toggle(void);

        void join(void);

    private:
        void _run(void);
        void _setup(void);
        void _init(void);
        float _scale_dpi();
        void _render(void);
        void _clear();

        /* Themes */
        void _render_frame(void);
        void _setup_rp(void);
        void _setup_behavior(void);
        void _setup_buttons(int width);

        void _apply_theme(void);

        GLFWwindow *_window = nullptr;
        Config &_config;
        std::thread _thread;

        std::atomic<bool> _show {false};
        std::atomic<bool> _hide {false};
        std::atomic<bool> _running {true};

        bool _tmp_show_title = true;
        bool _tmp_show_artist = true;
        bool _tmp_show_album = true;
        bool _tmp_show_cover = true;
        bool _tmp_show_timestamps = true;
        bool _tmp_auto_start = false;
};
