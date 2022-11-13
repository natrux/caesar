# Detect Clang libraries
#
# Defines the following variables:
#  CLANG_FOUND                 - True if Clang was found
#  CLANG_INCLUDE_DIRS          - Where to find Clang includes
#  CLANG_LIBRARY_DIRS          - Where to find Clang libraries
#  CLANG_BUILTIN_DIR           - Where to find Clang builtin includes
#
#  CLANG_CLANG_LIB             - Libclang C library
#
#  CLANG_CLANGFRONTEND_LIB     - Clang Frontend (C++) Library
#  CLANG_CLANGDRIVER_LIB       - Clang Driver (C++) Library
#  ...
#
#  CLANG_LIBS                  - All the Clang C++ libraries
#
# Uses the same include and library paths detected by FindLLVM.cmake
#
# See https://clang.llvm.org/docs/InternalsManual.html for full list of libraries

#=============================================================================
# Copyright 2014-2015 Kevin Funk <kfunk@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.

#=============================================================================

set(KNOWN_VERSIONS 8 7 6.0 5.0 4.0 3.9 3.8)

foreach(version ${KNOWN_VERSIONS})
    if (LLVM_DIR OR (DEFINED Clang_FIND_VERSION AND Clang_FIND_VERSION VERSION_GREATER version))
        break()
    endif ()

    find_package(LLVM ${version})
    if (LLVM_FOUND)
        break()
    endif ()
endforeach()

# if (${Clang_FIND_REQUIRED})
#     if(NOT DEFINED Clang_FIND_VERSION)
#         message(SEND_ERROR "Could not find Clang.")
#     else()
#         message("Found version ${Clang_FIND_VERSION}")
#     endif()
# endif()

set(CLANG_FOUND FALSE)

if (LLVM_LIBRARY_DIRS)
  if ("${OLD_LLVM_LIBRARY_DIRS}" STREQUAL "${LLVM_LIBRARY_DIRS}")
    set(CLEAR_LIBS FALSE)
  else()
    set(CLEAR_LIBS TRUE)
  endif()
  
  macro(FIND_AND_ADD_CLANG_LIB _libname_)
    string(TOUPPER ${_libname_} _prettylibname_)
    if (${CLEAR_LIBS})
      unset(CLANG_${_prettylibname_}_LIB)
    endif()
    find_library(CLANG_${_prettylibname_}_LIB NAMES ${_libname_} HINTS ${LLVM_LIBRARY_DIRS} ${ARGN} NO_DEFAULT_PATH)
    if (CLANG_${_prettylibname_}_LIB)
      set(CLANG_LIBS ${CLANG_LIBS} ${CLANG_${_prettylibname_}_LIB})
    endif()
  endmacro(FIND_AND_ADD_CLANG_LIB)
  
  FIND_AND_ADD_CLANG_LIB(clangFrontend)

  # note: On Windows there's 'libclang.dll' instead of 'clang.dll' -> search for 'libclang', too
  FIND_AND_ADD_CLANG_LIB(clang NAMES clang libclang) # LibClang: high-level C interface

  FIND_AND_ADD_CLANG_LIB(clangDriver)
  FIND_AND_ADD_CLANG_LIB(clangCodeGen)
  FIND_AND_ADD_CLANG_LIB(clangSema)
  FIND_AND_ADD_CLANG_LIB(clangChecker)
  FIND_AND_ADD_CLANG_LIB(clangAnalysis)
  FIND_AND_ADD_CLANG_LIB(clangRewriteFrontend)
  FIND_AND_ADD_CLANG_LIB(clangRewrite)
  FIND_AND_ADD_CLANG_LIB(clangAST)
  FIND_AND_ADD_CLANG_LIB(clangParse)
  FIND_AND_ADD_CLANG_LIB(clangLex)
  FIND_AND_ADD_CLANG_LIB(clangBasic)
  FIND_AND_ADD_CLANG_LIB(clangARCMigrate)
  FIND_AND_ADD_CLANG_LIB(clangEdit)
  FIND_AND_ADD_CLANG_LIB(clangFrontendTool)
  FIND_AND_ADD_CLANG_LIB(clangSerialization)
  FIND_AND_ADD_CLANG_LIB(clangTooling)
  FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCheckers)
  FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCore)
  FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerFrontend)
  FIND_AND_ADD_CLANG_LIB(clangRewriteCore)
  
  set(OLD_LLVM_LIBRARY_DIRS ${LLVM_LIBRARY_DIRS} CACHE INTERNAL "Last value used for LLVM_LIBRARY_DIRS")
endif()

if(CLANG_LIBS OR CLANG_CLANG_LIB)
  set(CLANG_FOUND TRUE)
else()
  message(STATUS "Could not find any Clang libraries in ${LLVM_LIBRARY_DIRS}")
endif()

if(CLANG_FOUND)
  set(CLANG_LIBRARY_DIRS ${LLVM_LIBRARY_DIRS})
  set(CLANG_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS})
  set(CLANG_VERSION ${LLVM_VERSION})

  # svn version of clang has a svn suffix "8.0.0svn" but installs the header in "8.0.0", without the suffix
  string(REPLACE "svn" "" CLANG_VERSION_CLEAN "${CLANG_VERSION}")

  find_path(CLANG_BUILTIN_DIR
            # cpuid.h because it is defined in ClangSupport constructor as valid clang builtin dir indicator
            NAMES "cpuid.h"
            PATHS "${CLANG_LIBRARY_DIRS}"
                  "${CLANG_INCLUDE_DIRS}"
            PATH_SUFFIXES "clang/${CLANG_VERSION}/include"
                          "../../../clang/${CLANG_VERSION}/include"
                          "clang/${CLANG_VERSION_CLEAN}/include"
                          "../../../clang/${CLANG_VERSION_CLEAN}/include"
            NO_DEFAULT_PATH
  )

  if (NOT CLANG_BUILTIN_DIR)
      message(FATAL_ERROR "Could not find Clang builtin directory")
  endif()
  get_filename_component(CLANG_BUILTIN_DIR ${CLANG_BUILTIN_DIR} ABSOLUTE)

  # check whether llvm-config comes from an install prefix
  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --src-root
    OUTPUT_VARIABLE _llvmSourceRoot
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  string(FIND "${LLVM_INCLUDE_DIRS}" "${_llvmSourceRoot}" _llvmIsInstalled)
  if (NOT _llvmIsInstalled)
    message(STATUS "Detected that llvm-config comes from a build-tree, adding more include directories for Clang")
    list(APPEND CLANG_INCLUDE_DIRS
         "${LLVM_INSTALL_PREFIX}/tools/clang/include" # build dir
         "${_llvmSourceRoot}/tools/clang/include"     # source dir
    )
  endif()

  message(STATUS "Found Clang (LLVM version: ${CLANG_VERSION})")
  message(STATUS "  Include dirs:        ${CLANG_INCLUDE_DIRS}")
  message(STATUS "  Clang libraries:     ${CLANG_LIBS}")
  message(STATUS "  Libclang C library:  ${CLANG_CLANG_LIB}")
  message(STATUS "  Builtin include dir: ${CLANG_BUILTIN_DIR}")
else()
  if(Clang_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Clang")
  endif()
endif()
