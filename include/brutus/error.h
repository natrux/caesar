#pragma once

#include <string>

#include <clang-c/CXErrorCode.h>
#include <clang-c/CXCompilationDatabase.h>


std::string cx_error_string(const CXErrorCode &error);
std::string cx_error_string(const CXCompilationDatabase_Error &error);


