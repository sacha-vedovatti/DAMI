/*
** PERSONNAL PROJECT, 2026
** DAMI
** File description:
** Graphical User Interface
*/

#include "Gui.hpp"

GUI::GUI(Config &config) : _config(config) {}

GUI::~GUI()
{
    _running = false;
    if (_window)
        glfwPostEmptyEvent();
    if (_thread.joinable())
        _thread.join();
}

bool GUI::init(void)
{
    _thread = std::thread(&GUI::_run, this);
    return true;
}

void GUI::show(void)
{
    _show = true;
    if (_window)
        glfwPostEmptyEvent();
}

void GUI::hide(void)
{
    _hide = true;
    if (_window)
        glfwPostEmptyEvent();
}

void GUI::toggle(void)
{
    if (_window && glfwGetWindowAttrib(_window, GLFW_VISIBLE))
        hide();
    else
        show();
}

void GUI::join(void)
{
    _running = false;
    if (_window)
        glfwPostEmptyEvent();
    if (_thread.joinable())
        _thread.join();
}

static void error_callback(int code, const char *desc)
{
    std::cerr << "[GLFW] Error " << code << ": " << desc << std::endl;
}

static void print_error(const std::string &message)
{
    std::cerr << "[GUI] Error: " << message << std::endl; 
}

void GUI::_run(void)
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return print_error("'glfwInit()' failed.");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow(480, 380, "DAMI – Settings", nullptr, nullptr);
    if (!_window) {
        glfwTerminate();
        return print_error("'glfwCreateWindow()' failed.");
    }
    _setup();
    _init();
    _apply_theme();
    _render();
    _clear();
}

void GUI::_setup(void)
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    glfwGetMonitorPos(monitor, &x, &y);
    width = mode ? mode->width : 480;
    height = mode ? mode->height : 380;
    glfwSetWindowPos(_window, x + (width - 480) / 2, y + (height - 380) / 2);
    glfwSetWindowCloseCallback(_window, [](GLFWwindow *w) {
        glfwSetWindowShouldClose(w, GLFW_FALSE);
        glfwHideWindow(w);
    });
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1);
}

float GUI::_scale_dpi()
{
    float DPI = 1.0f;
    int window_width = 0;
    int window_height = 0;
    int framebuffer_width = 0;
    int framebuffer_height = 0;

    glfwGetWindowSize(_window, &window_width, &window_height);
    glfwGetFramebufferSize(_window, &framebuffer_width, &framebuffer_height);
    if (window_width > 0 && window_height > 0) {
        float scale_x = (float) framebuffer_width / (float) window_width;
        float scale_y = (float) framebuffer_height / (float) window_height;
        DPI = scale_x > scale_y ? scale_x : scale_y;
    }
    if (DPI < 1.0f)
        DPI = 1.0f;
    return DPI;
}

void GUI::_init(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;

    float DPI = _scale_dpi();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f * DPI);
    if (io.Fonts->Fonts.empty())
        io.Fonts->AddFontDefault();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void GUI::_render(void)
{
    int width = 0;
    int height = 0;

    while (_running.load()) {
        if (_show.exchange(false)) {
            config_t *s = _config.get_settings();
            _tmp_show_title = s->show_title;
            _tmp_show_artist = s->show_artist;
            _tmp_show_album = s->show_album;
            _tmp_show_cover = s->show_cover;
            _tmp_show_timestamps = s->show_timestamps;
            _tmp_auto_start = s->auto_start;
            glfwShowWindow(_window);
            glfwFocusWindow(_window);
        }
        if (_hide.exchange(false))
            glfwHideWindow(_window);
        if (!glfwGetWindowAttrib(_window, GLFW_VISIBLE)) {
            glfwWaitEvents();
            continue;
        }
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        _render_frame();
        ImGui::Render();
        glfwGetFramebufferSize(_window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.114f, 0.125f, 0.157f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(_window);
    }
}

void GUI::_clear()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(_window);
    _window = nullptr;
    glfwTerminate();
}
