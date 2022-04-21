#pragma once

#include <string>
#include <cstdint>
#include <cstdarg>

std::string format_string(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
std::string format_string_v(const char* fmt, va_list);

std::string comma_num(const int64_t val); // comma_num(12345) == "12,345"
