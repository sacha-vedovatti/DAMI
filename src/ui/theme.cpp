/*
** EPITECH PROJECT, 2026
** DAMI
** File description:
** GUI themes
*/

#include "Gui.hpp"

static void _setup_header(void)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.949f, 0.961f, 0.980f, 1.0f));
    ImGui::SetWindowFontScale(1.25f);
    ImGui::Text("Apple Music Integration");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.612f, 0.647f, 0.714f, 1.0f));
    ImGui::Text("Settings");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void GUI::_setup_rp(void)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.345f, 0.392f, 0.886f, 1.0f)); // blurple label
    ImGui::Text("RICH PRESENCE");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Checkbox("Show title", &_tmp_show_title);
    ImGui::Checkbox("Show artist", &_tmp_show_artist);
    ImGui::Checkbox("Show album", &_tmp_show_album);
    ImGui::Checkbox("Show cover image", &_tmp_show_cover);
    ImGui::Checkbox("Show timestamps", &_tmp_show_timestamps);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void GUI::_setup_behavior(void)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.345f, 0.392f, 0.886f, 1.0f));
    ImGui::Text("BEHAVIOR");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Checkbox("Run on startup", &_tmp_auto_start);
    ImGui::Spacing();
    ImGui::Spacing();
}

void GUI::_setup_buttons(int width)
{
    config_t *settings = _config.get_settings();
    const float btn_w = 110.0f;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    float total = btn_w * 2 + spacing;

    ImGui::SetCursorPosX(((float) width - total) * 0.5f);
    if (ImGui::Button("Save", ImVec2(btn_w, 34))) {
        settings->show_title = _tmp_show_title;
        settings->show_artist = _tmp_show_artist;
        settings->show_album = _tmp_show_album;
        settings->show_cover = _tmp_show_cover;
        settings->show_timestamps = _tmp_show_timestamps;
        settings->auto_start = _tmp_auto_start;
        if (!_config.save())
            std::cerr << "[GUI] Failed to save config" << std::endl;
        else
            std::cout << "[CONFIG] Settings saved." << std::endl;
        glfwHideWindow(_window);
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.196f, 0.212f, 0.259f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.235f, 0.251f, 0.302f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.149f, 0.161f, 0.204f, 1.0f));
    if (ImGui::Button("Cancel", ImVec2(btn_w, 34))) {
        _tmp_show_title = settings->show_title;
        _tmp_show_artist = settings->show_artist;
        _tmp_show_album = settings->show_album;
        _tmp_show_cover = settings->show_cover;
        _tmp_show_timestamps = settings->show_timestamps;
        _tmp_auto_start = settings->auto_start;
        glfwHideWindow(_window);
    }
    ImGui::PopStyleColor(3);
}

void GUI::_render_frame(void)
{
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(_window, &width, &height);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float) width, (float) height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##root", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleVar();
    _setup_header();
    _setup_rp();
    _setup_behavior();
    _setup_buttons(width);
    ImGui::SetCursorPosY((float) height - 28.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.412f, 0.447f, 0.518f, 1.0f));
    ImGui::PopStyleColor();
    ImGui::End();
}

void GUI::_apply_theme(void)
{
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 6.0f;
    style.WindowPadding = ImVec2(18, 18);
    style.FramePadding = ImVec2(10,  6);
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2( 8,  6);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;

    const ImVec4 bg_dark = ImVec4(0.114f, 0.125f, 0.157f, 1.0f); // #1d2136 (main bg)
    const ImVec4 bg_mid = ImVec4(0.149f, 0.161f, 0.204f, 1.0f); // #262934 (panels)
    const ImVec4 bg_light = ImVec4(0.196f, 0.212f, 0.259f, 1.0f); // #323642 (hover/frames)
    const ImVec4 bg_lighter = ImVec4(0.235f, 0.251f, 0.302f, 1.0f); // #3c404d (active)
    const ImVec4 accent = ImVec4(0.345f, 0.392f, 0.886f, 1.0f); // #5865f2
    const ImVec4 accent_hov = ImVec4(0.412f, 0.463f, 0.941f, 1.0f); // #6975f0
    const ImVec4 accent_act = ImVec4(0.290f, 0.333f, 0.773f, 1.0f); // #4a55c5
    const ImVec4 text_prim = ImVec4(0.949f, 0.961f, 0.980f, 1.0f); // #f2f5fa
    const ImVec4 text_muted = ImVec4(0.612f, 0.647f, 0.714f, 1.0f); // #9ca5b6
    const ImVec4 border_col = ImVec4(0.255f, 0.275f, 0.337f, 1.0f); // #414656

    ImVec4 *c = style.Colors;
    c[ImGuiCol_WindowBg] = bg_dark;
    c[ImGuiCol_ChildBg] = bg_mid;
    c[ImGuiCol_PopupBg] = bg_mid;
    c[ImGuiCol_Border] = border_col;
    c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg] = bg_mid;
    c[ImGuiCol_FrameBgHovered] = bg_light;
    c[ImGuiCol_FrameBgActive] = bg_lighter;
    c[ImGuiCol_TitleBg] = bg_mid;
    c[ImGuiCol_TitleBgActive] = bg_mid;
    c[ImGuiCol_TitleBgCollapsed] = bg_dark;
    c[ImGuiCol_MenuBarBg] = bg_mid;
    c[ImGuiCol_ScrollbarBg] = bg_dark;
    c[ImGuiCol_ScrollbarGrab] = bg_lighter;
    c[ImGuiCol_ScrollbarGrabHovered] = accent;
    c[ImGuiCol_ScrollbarGrabActive] = accent_act;
    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accent_act;
    c[ImGuiCol_Button] = accent;
    c[ImGuiCol_ButtonHovered] = accent_hov;
    c[ImGuiCol_ButtonActive] = accent_act;
    c[ImGuiCol_Header] = bg_light;
    c[ImGuiCol_HeaderHovered] = bg_lighter;
    c[ImGuiCol_HeaderActive] = accent;
    c[ImGuiCol_Separator] = border_col;
    c[ImGuiCol_SeparatorHovered] = accent;
    c[ImGuiCol_SeparatorActive] = accent_act;
    c[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ResizeGripHovered] = accent;
    c[ImGuiCol_ResizeGripActive] = accent_act;
    c[ImGuiCol_Tab] = bg_mid;
    c[ImGuiCol_TabHovered] = accent_hov;
    c[ImGuiCol_TabActive] = accent;
    c[ImGuiCol_TabUnfocused] = bg_mid;
    c[ImGuiCol_TabUnfocusedActive] = bg_light;
    c[ImGuiCol_Text] = text_prim;
    c[ImGuiCol_TextDisabled] = text_muted;
    c[ImGuiCol_NavHighlight] = accent;
    c[ImGuiCol_NavWindowingHighlight]= accent;
}
