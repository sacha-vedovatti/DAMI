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
