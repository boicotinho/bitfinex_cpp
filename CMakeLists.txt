cmake_minimum_required(VERSION 2.8.12)
project(bitfinex_cpp)

# TODO: Make CMakeLists.txt just for wolfssl config
#set(WOLFSSL_TLS13 OFF)
#set(WOLFSSL_OPENSSLCOEXIST OFF)
#set(WOLFSSL_AESGCM OFF) # <<-- this is the cypher that disable Intel AES
set(WOLFSSL_OPENSSLEXTRA ON)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(extern/wolfssl)

# TODO: Make CMakeLists.txt just for libwebsockets config
# LIBWEBSOCKETS
set(DISABLE_WERROR ON )
set(LWS_WITH_STATIC ON )
set(LWS_WITH_SHARED OFF )
set(LWS_WITHOUT_TESTAPPS ON )
set(LWS_WITHOUT_TEST_SERVER ON )
set(LWS_WITHOUT_TEST_SERVER_EXTPOLL ON )
set(LWS_WITHOUT_TEST_PING ON )
set(LWS_WITHOUT_TEST_CLIENT ON )
set(LWS_WITH_WOLFSSL ON )
set(LWS_WOLFSSL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/wolfssl_include ) # copy otherwise CMake complains
set(LWS_WOLFSSL_LIBRARIES    ${CMAKE_CURRENT_BINARY_DIR}/extern/wolfssl/libwolfssl.a )
execute_process(COMMAND
    rsync -avru ${CMAKE_CURRENT_SOURCE_DIR}/extern/wolfssl/wolfssl ${LWS_WOLFSSL_INCLUDE_DIRS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE SSL_HDR_COPY_RES
)
message("@@@@@@@@@@@@@ SSL_HDR_COPY_RES         = ${SSL_HDR_COPY_RES}")
message("@@@@@@@@@@@@@ LWS_WOLFSSL_LIBRARIES    = ${LWS_WOLFSSL_LIBRARIES}")
message("@@@@@@@@@@@@@ LWS_WOLFSSL_INCLUDE_DIRS = ${LWS_WOLFSSL_INCLUDE_DIRS}")
add_subdirectory(extern/libwebsockets)


ADD_DEFINITIONS(-DBUILD_TYPE=\"${CMAKE_BUILD_TYPE}\")

set(CMAKE_CXX_STANDARD 11)

#-march=native -mtune=native # core-avx2
# Compile settings which potentially changes the ABI
# should come from top-level CMakeFile or alternatively,
# anything ABI-sensitive should be wrapped by pImpl pattern.
add_compile_options(
    -Wall -Wextra -Wpedantic -Werror
    -Wdeprecated -W -Wpointer-arith -Wno-uninitialized -Wno-unused
    -Wno-sign-compare -Wno-comment -Wno-unknown-pragmas -Wno-unused-parameter
    -Wno-invalid-offsetof -no-integrated-cpp -fno-strict-aliasing
    -Werror -Werror=return-type
    -fPIC
)

include_directories(SYSTEM .)

add_subdirectory(core)
add_subdirectory(web_socket)
add_subdirectory(marketlinks)
