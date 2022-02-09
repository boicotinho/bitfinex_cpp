#include "url.h"
#include <regex>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

constexpr char const* WebSocketURL::DEFAULT_WSS;
constexpr char const* WebSocketURL::DEFAULT_WS;

// ws://websocket-echo.com          // port 80,  ssl=0, hand_shake = /
// wss://api-pub.bitfinex.com/ws/2  // port 433, ssl=1, hand_shake = /ws/2
// ws://websocket-echo.com:9999/xx  // port 999, ssl=0, hand_shake = /xx
WebSocketURL::WebSocketURL(std::string const& a_url)
{
    // https://www.regextester.com/97722
    std::regex g_rgx(
        R"((wss?):\/\/((www\.)?[-a-zA-Z0-9@%._\+~#=]{1,256})(:[0-9]*)?([-a-zA-Z0-9()@:%_\+.~#?&//=]*))" );

    std::smatch pieces_match;

    if(!std::regex_match(a_url, pieces_match, g_rgx))
        throw std::runtime_error("Bad websocket URL: " + a_url);

    if(pieces_match.size() != 6)
        throw std::runtime_error("Websocket URL regex failed: '" + a_url + "'");

    const auto fullm  = pieces_match[0].str(); // ws://www.websocket-echo.com:333/xx
    const auto proto  = pieces_match[1].str(); // ws
    const auto zhost  = pieces_match[2].str(); // www.websocket-echo.com
    const auto domain = pieces_match[3].str(); // www.
    const auto zport  = pieces_match[4].str(); // :333
    const auto path   = pieces_match[5].str(); // /xx

    const std::string defport = std::string(proto == "wss" ? DEFAULT_WSS : DEFAULT_WS);

    this->use_ssl = proto == "wss";
    this->host = zhost;
    this->port = zport.empty() ? defport : zport.substr(1);
    this->hand_shake = path.empty() ? "/" : path;

    // for (size_t ii = 0; ii < pieces_match.size(); ++ii)
    // {
    //     std::ssub_match sub_match = pieces_match[ii];
    //     std::string piece = sub_match.str();
    //     std::cout << "  submatch " << ii << ": " << piece << '\n';
    // }
}
