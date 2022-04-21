#include "web_socket_simple/web_socket_client.h"
#include "core/profile_utils.h"
#include "core/string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <limits>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>

using namespace std;
using namespace std::chrono;
using Clock = high_resolution_clock;

// {"result":null,"id":1}
// {"u":18511620617,"s":"BTCUSDT","b":"41233.75000000","B":"0.80879000","a":"41233.76000000","A":"2.36324000"}
// {"u":18511620621,"s":"BTCUSDT","b":"41233.75000000","B":"0.77706000","a":"41233.76000000","A":"2.36324000"}

// Negotiated cypher (server hello):
// Cipher Suite Name (OpenSSL)	Key Exchange	Encryption	Key Length
// ECDHE-RSA-AES128-GCM-SHA256	ECDH 256	    AES GCM	    128

// openssl speed aes    (482 ns for 64bytes, likely 1us for a ticker message)
// Doing aes-128 cbc for 3s on 64 size blocks: 6227098 aes-128 cbc's in 3.00s
// Doing aes-192 cbc for 3s on 64 size blocks: 5317465 aes-192 cbc's in 3.00s
// Doing aes-256 cbc for 3s on 64 size blocks: 4621699 aes-256 cbc's in 3.00s

// objdump -d /lib/x86_64-linux-gnu/libcrypto.so.1.1
// AES_unwrap_key@@OPENSSL_1_1_0 --> uses aesenc (avx instruction set)

int main(int argc, const char** argv)
{
    WebSocketURL parsed_url("wss://52.199.70.71:9443/ws"); // stream.binance.com
    auto         subscription = "btcusdt@bookTicker"; // "btcusdt@trade"
    uint32_t     pool_size    = std::thread::hardware_concurrency();
    auto         offset_time  = seconds(10);
    auto         common_time  = seconds(20);
    size_t       reserve_size = common_time.count() * 20;
    bool         print        = false;

    if(argc > 1)
        offset_time = seconds(atoi(argv[1]));

    if(argc > 2)
        common_time = seconds(atoi(argv[2]));

    if(argc > 3)
        pool_size = atoi(argv[3]);

    if(argc > 4)
        parsed_url.host = argv[4];

    if(argc > 5)
        subscription = argv[5];

    cerr << "Analyzing: " << parsed_url.host
         << " subscription: " << subscription
         << " pool_size: " << pool_size
         << '\n';

    auto const rpc_req = format_string(
        R"({"method": "SUBSCRIBE", "params": ["%s"], "id": 1})", subscription);

    atomic_int  left_to_start {(int)pool_size};
    atomic_int  left_to_perf  {(int)pool_size};
    atomic_bool quit {false};
    mutex       mtx;

    struct Update
    {
        uint64_t        id;
        CpuTimeStamp    ts;
    };

    struct Result
    {
        std::thread     m_thread;
        vector<Update>  m_updates;
        vector<Update>::iterator m_it;
        vector<int64_t> m_latency;
        size_t          m_winner_count {};
        char            m_padding[64];
    };

    vector<Result> threads (pool_size);

    auto fn_thread = [&](size_t ix)
    {
        try
        {
            Result& a_result = threads[ix];
            a_result.m_updates.reserve(reserve_size);

            --left_to_start;
            while(left_to_start)
            {}

            std::this_thread::sleep_for( offset_time * ix);

            WebSocketClient ws_client = WebSocketClient(parsed_url);

            //if(print)
                cerr << "Thread " << ix << " connected.\n";

            --left_to_perf;
            if(!left_to_perf)// && print)
                cerr << "Signal threads to start profiling.\n";

            ws_client.blk_send_str(rpc_req);
            {
                auto const buf = ws_client.consume_begin();
                string response(buf.data, buf.len);
                if(response != R"({"result":null,"id":1})")
                    throw std::runtime_error("Unexpected response: "+response);
                ws_client.consume_commit(buf.len);
            }

            uint64_t last_uid = 0;

            while(!quit)
            {
                auto const buf = ws_client.consume_begin();
                auto const ts = rdtscp();
                // {"u":18511620617,"s":"BTCUSDT","b":"41233.75000000",...
                uint64_t const update_id = std::atol(buf.data + 5);
                if(left_to_perf.load(memory_order_acquire) == 0
                    && update_id > last_uid )
                {
                    a_result.m_updates.push_back(Update{update_id, ts});
                    last_uid = update_id;
                }
                if(print)
                {
                    string data(buf.data, buf.len);
                    cerr << data << "\n";
                }
                ws_client.consume_commit(buf.len);
            }
            unique_lock<mutex> lk(mtx);
            cerr << "Thread " << ix << " stopped, profiled: "
                 << a_result.m_updates.size() << "\n";
        }
        catch(const std::exception& ex)
        {
            cerr << "Inner thread exception: " << ex.what() << '\n';
        }
    };

    for(size_t ii = 0; ii < threads.size(); ++ii)
        threads[ii].m_thread = std::thread { fn_thread, ii };

    while(left_to_start)
    {}
    std::this_thread::sleep_for(common_time + offset_time * pool_size);
    quit = true;

    for(Result& tt : threads)
    {
        tt.m_thread.join();
        tt.m_it = tt.m_updates.begin();
    }

    uint64_t curr_upd_id = 0;
    for(Result& tt : threads)
    {
        if(tt.m_it == tt.m_updates.end())
            goto DISPLAY_RESULTS;
        curr_upd_id = max(curr_upd_id, tt.m_it->id);
    }

    for(Result& tt : threads)
    {
        for(;;++tt.m_it)
        {
            if(tt.m_it == tt.m_updates.end())
                goto DISPLAY_RESULTS;
            if(tt.m_it->id == curr_upd_id)
                break;
        }
    }

    while(threads[0].m_it != threads[0].m_updates.end())
    {
        curr_upd_id = threads[0].m_it->id;
        CpuTimeStamp best_ts = threads[0].m_it->ts;

        for(Result& tt : threads)
        {
            if(tt.m_it == tt.m_updates.end() || tt.m_it->id != curr_upd_id)
                goto DISPLAY_RESULTS;
            best_ts = min(best_ts, tt.m_it->ts);
        }

        for(Result& tt : threads)
        {
            auto const lat = tt.m_it->ts - best_ts;
            tt.m_latency.push_back(lat);
            tt.m_winner_count += lat == 0;
            ++tt.m_it;
        }
    }

DISPLAY_RESULTS:

    size_t const num_samples = threads[0].m_latency.size();
    cerr << "Number of samples: " << num_samples << '\n';

    for(size_t ii = 0; ii < num_samples; ++ii)
    {
        for(Result& tt : threads)
        {
            cout << tt.m_latency[ii] << ',';
        }
        cout << '\n';
    }

    for(Result& tt : threads)
        cerr << "Winner count: " << tt.m_winner_count << "\n";

    return EXIT_SUCCESS;
}
