#pragma once
#include "core/str_view.h"
#include <string>
#include <vector>
#include <stdint.h>

class RestRequest
{
    std::string m_msg;
public:
    enum class eKeyword {Null, True, False};
    RestRequest& set(std::string key, std::string); // will add "quotes"
    RestRequest& set(std::string key, std::vector<std::string>); // will add ["", ""]
    RestRequest& set(std::string key, int64_t);
    RestRequest& set(std::string key, double);
    RestRequest& set(std::string key, eKeyword);

    BufDesc get_message() const;

    //using response_token = uint32_t;
};
