# boost / asio / beast / openssl

//Centos:        sudo yum install -y openssl-devel
//Ubuntu/Debian: sudo apt install libssl-dev

I had boost 1.76 installed on my centos8

git clone --recursive https://github.com/boostorg/boost.git   # 28 minutes
cd boost
./bootstrap.sh --prefix=/home/fabio/dev/dist/ # 22 seconds
./b2 # 2 minutes

      --prefix=/home/fabio/dev/dist/
  include dir: /home/fabio/dev/boost
  lib dir:     /home/fabio/dev/boost/stage/lib

./b2 install

# wscat, TOB

sudo yum install -y npm
sudo npm install -g wscat

wscat -c wss://api-pub.bitfinex.com/ws/2
{ "event": "subscribe", "channel": "book", "prec": "R0", "symbol": "tBTCUSD" }

wscat -c wss://api-pub.bitfinex.com/ws/2
{ "event": "subscribe", "channel": "book", "symbol": "tBTCUSD" }

# Current TOB, to check real data against

Bitfinex Websocket API version is 2.0

https://www.bitfinex.com/

BTC/USD
Price: integer e.g. 41625. Price tick = 1 USD
Ammount: the total sum of quantity (BTC, a fixed-point vale) only at this price level
Total: cumulative quantity from this price level to top of book
Count: number of orders at this price level only

# BOOKS

Raw (R0)
https://docs.bitfinex.com/reference#ws-public-raw-books

Symbols
https://docs.bitfinex.com/v1/reference#rest-public-symbols
https://api.bitfinex.com/v1/symbols_details

Python WebSocket example, exchange
https://bitfinex.readthedocs.io/en/latest/websocket.html#wssclient-with-examples

# C++ WebSockets example

https://stackoverflow.com/questions/69051106/c-or-c-websocket-client-working-example

Header-only but also uses asio:
https://github.com/zaphoyd/websocketpp
A bit thick framework, might be hard to do asynchronous, define your own buffers and allocations, etc
Users can't make the important decisions such as buffer or thread management.

// Stand-alon boost/beast?
https://github.com/boostorg/beast

// No boost dependency, simple, SSL, but forces a threading framework
https://github.com/machinezone/IXWebSocket

// asynchronous / synchronous
https://www.boost.org/doc/libs/develop/libs/beast/doc/html/beast/examples.html
https://www.boost.org/doc/libs/develop/libs/beast/example/websocket/client/sync-ssl/websocket_client_sync_ssl.cpp

    root_certificates.hpp : better use this certificate chain validator:
    https://stackoverflow.com/questions/49507407/using-boost-beast-asio-http-client-with-ssl-https


# Order book initial state

"P0" subscription, first message:
// on trading pairs (ex. tBTCUSD)
[
  CHANNEL_ID,
  [
    [
      PRICE,
      COUNT,
      AMOUNT
    ],
    ...
  ]
]


[PRICE, num total orders, total ammount (negative qty means sell)]
[
[54146, 1, 0.0148], // BEST BID, price level
[54144, 1, 0.046164],
[54143, 1, 0.092318],
[54142, 1, 0.02992778],
[54132, 1, 0.1],
[54131, 1, 0.03708065],
[54129, 2, 0.23899395],
[54127, 2, 0.10679801],
[54124, 1, 0.5],
[54120, 5, 1.87937266],
[54119, 1, 0.2797],
[54118, 2, 0.168488],
[54116, 1, 0.12934954],
[54110, 1, 0.00125672],
[54109, 1, 0.3689],
[54108, 1, 0.7131],
[54107, 1, 0.00281756],
[54106, 2, 0.10010968],
[54105, 4, 0.38591798],
[54104, 1, 0.07388],
[54103, 1, 0.04646],
[54102, 1, 0.184622],
[54098, 1, 1.0268],
[54097, 2, 1.5666],
[54096, 1, 0.7456],

[54147, 4, -1.63728786], // BEST ASK, price level
[54149, 1, -0.02],
[54150, 1, -0.00057112],
[54153, 1, -0.08563674],
[54155, 1, -0.09288401],
[54156, 1, -0.12437904],
[54158, 1, -0.2],
[54160, 1, -0.00207892],
[54162, 2, -0.01163271],
[54163, 2, -1.8178],
[54165, 1, -0.05197293],
[54168, 2, -1.5743],
[54169, 1, -0.02],
[54170, 2, -0.184626],
[54171, 1, -0.10394585],
[54173, 1, -0.13848181],
[54174, 1, -0.092322],
[54176, 1, -0.15591878],
[54177, 1, -0.138482],
[54182, 2, -2.055],
[54183, 1, -0.007933],
[54185, 1, -0.3],
[54187, 2, -0.39252692],
[54189, 2, -0.481572],
[54190, 1, -0.184636]]