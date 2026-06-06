cmake_minimum_required(VERSION 3.20)
project(DAMI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
)

add_executable(DAMI WIN32
    ${SRC_FILES}
)

target_include_directories(DAMI PRIVATE src)
target_compile_definitions(DAMI PRIVATE UNICODE _UNICODE)

target_link_libraries(DAMI
    windowsapp
    winhttp
    ws2_32
    Shell32
    Comctl32
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
