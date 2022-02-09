#pragma once
#include <string>

struct WebSocketURL
{
    std::string host        {"127.0.0.1"};
    std::string port        {DEFAULT_WSS}; // 80 for non-encrypted
    std::string hand_shake  {"/"};  // /ws/2
    bool        use_ssl     {DEFAULT_WSS == port};

    constexpr static char const* DEFAULT_WSS = "443";
    constexpr static char const* DEFAULT_WS  = "80";

    WebSocketURL() = default;
    WebSocketURL(std::string const& url);
};
