#include "ws_client.h"

WsClient& WsClient::swp(WsClient& other)
{
    std::swap(m_impl, other.m_impl);
    return *this;
}
