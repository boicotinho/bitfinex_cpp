#pragma once
#include "core/worker_thread.h"
#include <vector>
#include <unordered_map>

class ServerEngine
{
public:
    struct PrototypeTag {};
    ServerEngine(PrototypeTag);
    ServerEngine() = default;
    ~ServerEngine();

    // get_marketlink("binance");
    //   ->get_book("btcusdt")
    //
    // create_marker(cloid, side, price_delta, callback, books=[]) -> Marker

private:
    std::vector<WorkerThread> m_threads;
};
