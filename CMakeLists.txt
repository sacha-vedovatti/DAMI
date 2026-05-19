cmake_minimum_required(VERSION 3.20)
project(DAMI)

set(CMAKE_CXX_STANDARD 20)

file(GLOB DISCORD_GAME_SDK_CPP_SOURCES CONFIGURE_DEPENDS
	libs/discord_game_sdk/cpp/*.cpp
)

add_executable(DAMI
	src/main.cpp
	src/cover/cover.cpp
	src/rpc/discord.rpc.cpp
	src/music/music.cpp
	${DISCORD_GAME_SDK_CPP_SOURCES}
)

target_link_libraries(DAMI
	windowsapp
	winhttp
)

target_include_directories(DAMI PRIVATE
	libs/discord_game_sdk/cpp
)

target_link_libraries(
	DAMI
	${CMAKE_SOURCE_DIR}/libs/discord_game_sdk/lib/x86_64/discord_game_sdk.dll.lib
)

add_custom_command(TARGET DAMI POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/libs/discord_game_sdk/lib/x86_64/discord_game_sdk.dll"
		"$<TARGET_FILE_DIR:DAMI>"
)
