cmake_minimum_required(VERSION 3.20)
project(DAMI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_FILES
    src/main.cpp
    src/config/Config.cpp
    src/music/Music.cpp
    src/rpc/DiscordRPC.cpp
    src/server/Server.cpp
    src/tray/TraySys.cpp
    src/ui/Gui.cpp
    src/ui/theme.cpp
)

add_subdirectory(external/glfw)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(DAMI WIN32
    ${SRC_FILES}
    ${IMGUI_SRC}
    assets/DAMI.rc
)

target_include_directories(DAMI PRIVATE
    src
    src/config
    external/imgui
    external/imgui/backends
)

target_link_libraries(DAMI
    glfw
    opengl32
    winhttp
    ws2_32
    runtimeobject
    windowsapp
)

if(DEFINED ENV{DISCORD_CLIENT_ID})
    add_compile_definitions(COMPILED_DISCORD_CLIENT_ID=$ENV{DISCORD_CLIENT_ID})
else()
    message(WARNING "DISCORD_CLIENT_ID not set.")
endif()

if(MSVC)
    target_link_options(DAMI PRIVATE
        "/MANIFESTUAC:level='asInvoker' uiAccess='false'"
    )
endif()

add_custom_command(TARGET DAMI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:DAMI>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/DAMI.ico"
        "$<TARGET_FILE_DIR:DAMI>/DAMI.ico"
)
