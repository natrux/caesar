project(testcode)
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall -Wextra)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_library(dings SHARED
	src/Dings.cpp
	src/Dongs.cpp
)
target_include_directories(dings PUBLIC
	include/
)


add_executable(main
	src/main.cpp
)

target_link_libraries(main
	dings
)


