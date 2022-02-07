#pragma once

#include <string>
#include <cstdint>
#include <cstdarg>

std::string FormatString(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
std::string FormatStringV(const char* fmt, va_list);

std::string CommaNum(const int64_t val); // CommaNum(12345) == "12,345"
