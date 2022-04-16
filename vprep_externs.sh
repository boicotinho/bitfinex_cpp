clear
### WolfSSL
### Note 2
# wolfSSL takes a different approach to certificate verification than OpenSSL
# does. The default policy for the client is to verify the server, this means
# that if you don't load CAs to verify the server you'll get a connect error,
# no signer error to confirm failure (-188).
#
# If you want to mimic OpenSSL behavior of having `SSL_connect` succeed even if
# verifying the server fails and reducing security you can do this by calling:
#
# ```c
# wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_NONE, NULL);
# ```
#
# before calling `wolfSSL_new();`. Though it's not recommended.

pushd extern/wolfssl
#./autogen.sh
#./configure --enable-opensslextra --enable-static=yes --enable-shared=no
make -j
#make check
popd

# Lib output : libwolfssl.so.32.0.0* (a 64-bit binary, despite the name)
# wolfss has an openssl mimic layer
#ls -lhrt extern/wolfssl/src/.libs/
WOLFSSL_LIB=/home/fabio/dev/market_py2cpp/extern/bitfinex_cpp/extern/wolfssl/src/.libs/libwolfssl.a
WOLFSSL_HDR=/home/fabio/dev/market_py2cpp/extern/bitfinex_cpp/extern/wolfssl

file ${WOLFSSL_LIB}
file ${WOLFSSL_HDR}

# TODO: Need to enable: Fast RSA, ENABLED_FAST_RSA
# TODO: -march=native -mtune=native

#cmake .. -DLWS_WITH_WOLFSSL=1 \
#     -DLWS_WOLFSSL_INCLUDE_DIRS=/path/to/wolfssl \
#     -DLWS_WOLFSSL_LIBRARIES=/path/to/wolfssl/wolfssl.a ..

# https://libwebsockets.org/lws-api-doc-main/html/md_READMEs_README_build.html
# async DNS:
# https://libwebsockets.org/lws-api-doc-main/html/md_READMEs_README_async_dns.html

# TODO: when including wss cmake:
# LWS_STATIC_PIC
#   set(CMAKE_POLICY_DEFAULT_CMP<CMP0024> NEW)
#   -DCMAKE_POLICY_DEFAULT_CMP0024=OLD
pushd extern/libwebsockets
rm -rf   build_externs
mkdir -p build_externs
cd       build_externs
cmake ../ \
    -DDISABLE_WERROR=ON \
    -DLWS_WITH_STATIC=ON \
    -DLWS_WITH_SHARED=OFF \
    -DLWS_WITHOUT_TESTAPPS=ON \
    -DLWS_WITHOUT_TEST_SERVER=ON \
    -DLWS_WITHOUT_TEST_SERVER_EXTPOLL=ON \
    -DLWS_WITHOUT_TEST_PING=ON \
    -DLWS_WITHOUT_TEST_CLIENT=ON \
    -DLWS_WITH_WOLFSSL=ON \
    -DLWS_WOLFSSL_INCLUDE_DIRS=${WOLFSSL_HDR} \
    -DLWS_WOLFSSL_LIBRARIES=${WOLFSSL_LIB}
make -j
popd

file extern/libwebsockets/build_externs/lib/libwebsockets.a
