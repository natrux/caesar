cmake_minimum_required(VERSION 3.16)
project(caesar)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(
	-Wall -Wextra
	-Wsuggest-override
	-Wno-cpp                        # https://gitlab.gnome.org/GNOME/gtksourceviewmm/-/issues/3

	#-Werror
)
add_compile_options(
	# https://gitlab.gnome.org/GNOME/gtksourceview/-/issues/84
	-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_60

	# if your build fails because of glib version, comment out the above line and uncomment the following one.
	#-Wno-deprecated-declarations
)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
LIST(APPEND CMAKE_MODULE_PATH
	${CMAKE_SOURCE_DIR}/cmake
)

find_package(Clang REQUIRED)
find_package(LLVM REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK3 REQUIRED gtkmm-3.0)
pkg_check_modules(GSV3 REQUIRED gtksourceviewmm-3.0)


add_library(brutus SHARED
	src/Index.cpp
	src/CompilationDatabase.cpp
	src/CXTranslationUnit_Wrapper.cpp
	src/availability_e.cpp
	src/string.cpp
	src/source_location_t.cpp
	src/source_range_t.cpp
	src/error.cpp
	src/diagnostic_severity_e.cpp
	src/diagnostic_t.cpp
	src/completion_t.cpp
	src/completion_chunk_t.cpp
	src/fixit_t.cpp
	src/cursor_t.cpp
)

target_include_directories(brutus PUBLIC
	${CLANG_INCLUDE_DIRS}
	include/
)

target_link_libraries(brutus
	${CLANG_CLANG_LIB}
)


add_library(pompeius SHARED
	src/TranslationUnit.cpp
	src/ASTReferences.cpp
	src/TUParser.cpp
	src/TUServer.cpp
	src/Project.cpp
	src/File.cpp
	src/ThreadPool.cpp
)

target_include_directories(pompeius PUBLIC
	include/
)

target_link_libraries(pompeius
	brutus
	Threads::Threads
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	list(APPEND CRASSUS_PLATFORM_SOURCES
		src/linux/ls.cpp
	)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	list(APPEND CRASSUS_PLATFORM_SOURCES
		src/win32/ls.cpp
	)
#elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	#macos
else()
	message(FATAL_ERROR "Operating System might not be supported, remove this check at your own risk.")
endif()

add_library(crassus SHARED
	${CRASSUS_PLATFORM_SOURCES}
	src/MainWindow.cpp
	src/ProjectTab.cpp
	src/FileTab.cpp
	src/TabLabel.cpp
	src/DirectoryTreeView.cpp
	src/tag_properties_t.cpp
	src/CompletionProvider.cpp
	src/uri.cpp
	src/strings.cpp
	src/textbuffer.cpp
)

target_include_directories(crassus PUBLIC
	include/
	${GTK3_INCLUDE_DIRS}
	${GSV3_INCLUDE_DIRS}
)

target_link_libraries(crassus
	pompeius
	${GTK3_LIBRARIES}
	${GSV3_LIBRARIES}
)


add_executable(caesar
	src/main.cpp
)

target_link_libraries(caesar
	pompeius
	brutus
	crassus
)


add_executable(test
	src/test.cpp
)

target_link_libraries(test
	brutus
	pompeius
)

