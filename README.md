
Create a python module in C++, which subscribes (in a separate thread) to Bitfinex order book updates using websockets, keeping the order book state in memory.
There should be a pythonic FFI interface of this module, which lets you get the best bid and ask from the current order book.
Example of usage:

from bitfinex_quotes import Quotes # this should be implemented in C++

quotes = Quotes("BTCUSD")
quotes.subscribe()
for _ in range(10):
    sleep(1)
    print('best_bid', quotes.best_bid_price())
    print('best_ask', quotes.best_ask_price())
    print('best_bid', quotes.best_bid_volume())
    print('best_ask', quotes.best_ask_volume())

quotes.unsubscribe()

Also, please note that the most usual operations that will be needed from Quotes are get best bid/ask price, get best bid/ask volume, check the volume at a current price level and get best N levels. 

We also often ask the quotes the following question: 
    find a price level that has at least X volume on levels that are equal or better than this level
    (better meaning higher for bid and lower for ask).

For this exercise, you don't need to implement all these methods, only get best price and get best volume, but these possible usages are something to keep in mind when choosing a proper data structure. (For example, for best bid and best ask in theory you could just store the best levels and that's it, but this will not work since we'll need to do more operations with the orderbook in the future.

In regards to what you should target for this code - elegance or speed/efficiency, we will leave it at your best judgment. Adhere more or less to the same standards that you would if we were to start working together. But obviously, there's no need for some crazy super low-level optimizations.

Here's a link to the docs:

https://docs.bitfinex.com/reference#ws-public-books

======================================================================================================================================

1. Symbol details: minimum_margin and initial_margin, this is for margin trading and has nothing to do with the order book. Price precision we calculate based on quotes that we receive and mantissa. Other parameters are described in docs
https://docs.bitfinex.com/v1/reference#rest-public-symbol-details.
For this exercise you can just hardcode some tick sizes, for example, 1 for btc/usd and 0.1 for eth/usd. On our side, we have a simple function that assigns tick sizes for Bitfinex based on the current price (some exchanges provide this info, but I believe Bitfinex doesn't)

2. There's no lot size on spot crypto trading usually, there's just the minimum order volume (usually around $20 give or take).

3. s in wss stands for secured. WS over TLS. We should probably ask, if there is a way to get this data without encryption I suppose, as it might help using methods you mentioned

4. Documentation can be tricky, it’s always better to just connect and see what we receive. To give you an example of Bitfinex quotes subscription, the first message that we receive represents a snapshot of the order book (25 bids, 25 asks - negative volumes), the next ones are updates to this order book.
Example (each new line (or usually new array) is a new message).
[[54146, 1, 0.0148], [54144, 1, 0.046164], [54143, 1, 0.092318], [54142, 1, 0.02992778], [54132, 1, 0.1], [54131, 1, 0.03708065], [54129, 2, 0.23899395], [54127, 2, 0.10679801], [54124, 1, 0.5], [54120, 5, 1.87937266], [54119, 1, 0.2797], [54118, 2, 0.168488], [54116, 1, 0.12934954], [54110, 1, 0.00125672], [54109, 1, 0.3689], [54108, 1, 0.7131], [54107, 1, 0.00281756], [54106, 2, 0.10010968], [54105, 4, 0.38591798], [54104, 1, 0.07388], [54103, 1, 0.04646], [54102, 1, 0.184622], [54098, 1, 1.0268], [54097, 2, 1.5666], [54096, 1, 0.7456], [54147, 4, -1.63728786], [54149, 1, -0.02], [54150, 1, -0.00057112], [54153, 1, -0.08563674], [54155, 1, -0.09288401], [54156, 1, -0.12437904], [54158, 1, -0.2], [54160, 1, -0.00207892], [54162, 2, -0.01163271], [54163, 2, -1.8178], [54165, 1, -0.05197293], [54168, 2, -1.5743], [54169, 1, -0.02], [54170, 2, -0.184626], [54171, 1, -0.10394585], [54173, 1, -0.13848181], [54174, 1, -0.092322], [54176, 1, -0.15591878], [54177, 1, -0.138482], [54182, 2, -2.055], [54183, 1, -0.007933], [54185, 1, -0.3], [54187, 2, -0.39252692], [54189, 2, -0.481572], [54190, 1, -0.184636]]
[54144, 0, 1]
[54143, 0, 1]
[54119, 0, 1]
[54110, 0, 1]
[54171, 0, -1]
[54174, 0, -1]
[54190, 0, -1]
[54146, 1, 0.00052381]
[54133, 1, 0.29256808]
[54122, 1, 0.2797]
[54118, 1, 0.138488]
[54115, 1, 0.03]
[54111, 1, 0.461589]
[54109, 2, 0.37853889]
[54097, 3, 2.489813]
[54160, 1, -0.00207864]
[54162, 2, -0.01163279]
[54165, 1, -0.05196595]
[54170, 1, -0.10393189]
[54176, 1, -0.15592001]
[54183, 2, -0.89352256]
[54184, 2, -0.153282]
[54187, 1, -0.20789335]
[54191, 4, -0.94844886]
[54193, 2, -0.73080964]
[54146, 0, 1]
[54132, 0, 1]
[54131, 0, 1]
[54129, 0, 1]
[54124, 0, 1]
[54122, 0, 1]
[54115, 0, 1]
[54111, 0, 1]
[54107, 0, 1]
[54104, 0, 1]
[54103, 0, 1]
[54102, 0, 1]
[54165, 0, -1]
[54170, 0, -1]
[54176, 0, -1]
[54177, 0, -1]
[54187, 0, -1]
[54134, 1, 0.0148]
[54133, 1, 0.5]
[54127, 1, 0.00120755]
[54120, 2, 0.30837266]
[54119, 2, 0.14855824]
[54116, 3, 0.29900683]
[54109, 1, 0.3689]

We use this parameters to subscribe to a feed, (prevision P0, Length 25):
subscription_message = {'event': 'subscribe', 'channel': 'book', 'symbol': symbol, 'prec': 'P0'}

About length - length just stands for “depth” of the order book in this case. We need usually 5, so the lowest we can get here is length:25. We receive snapshots and updates for 25 bids and 25 asks (so a total of 50 price levels).

5. Bitfinex provides some code that shows how to connect to their exchange, you can check it out:
https://bitfinex.readthedocs.io/en/latest/_modules/bitfinex/websockets/client.html
