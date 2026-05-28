cmake_minimum_required(VERSION 3.20)
project(DAMI)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
)

add_executable(DAMI
    ${SRC_FILES}
)

target_link_libraries(DAMI
	windowsapp
	winhttp
	ws2_32
)

if(DEFINED ENV{DISCORD_CLIENT_ID})
    add_compile_definitions(COMPILED_DISCORD_CLIENT_ID=$ENV{DISCORD_CLIENT_ID})
else()
    message(WARNING "DISCORD_CLIENT_ID not set, will use config.json fallback")
endif()
