#pragma once
#include <stddef.h>

struct CBufDesc
{
    char const* data;
    size_t      len;
};

struct BufDesc // alias std::string_view?
{
    char*  data;
    size_t len;
    CBufDesc cbuf() const {return {data, len}; }
    operator CBufDesc() const {return {data, len}; }
};
