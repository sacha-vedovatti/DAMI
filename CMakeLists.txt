cmake_minimum_required(VERSION 3.20)
project(DAMI)

set(CMAKE_CXX_STANDARD 20)

add_executable(DAMI
	src/main.cpp
	src/cover/cover.cpp
	src/rpc/discord.rpc.cpp
	src/music/music.cpp
)

target_link_libraries(DAMI
	windowsapp
	winhttp
)
