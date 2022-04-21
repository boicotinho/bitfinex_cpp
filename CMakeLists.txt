cmake_minimum_required(VERSION 2.8.12)
project(bitfinex_cpp)

# Same as libwebsockets
#SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
#SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
#SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

#===============================================================================
# WolfSSL
#===============================================================================
# TODO: Make CMakeLists.txt just for wolfssl config
#set(WOLFSSL_TLS13 OFF)
#set(WOLFSSL_OPENSSLCOEXIST OFF)
#set(WOLFSSL_AESGCM OFF) # <<-- this is the cypher that disables Intel AES
set(WOLFSSL_OPENSSLEXTRA ON)
set(BUILD_SHARED_LIBS OFF)
set(WOLFSSL_CRYPT_TESTS OFF)
add_subdirectory(extern/wolfssl)

#===============================================================================
# LIBWEBSOCKETS
#===============================================================================
# TODO: Make CMakeLists.txt just for libwebsockets config
# set(LWS_ROLE_WS=1)
# set(LWS_WITH_SECURE_STREAMS=1)
# set(LWS_WITHOUT_EXTENSIONS=0)
#----
set(DISABLE_WERROR ON )
set(LWS_WITH_STATIC ON )
set(LWS_WITH_SHARED OFF )
set(LWS_WITHOUT_TESTAPPS ON )
set(LWS_WITHOUT_TEST_SERVER ON )
set(LWS_WITHOUT_TEST_SERVER_EXTPOLL ON )
set(LWS_WITHOUT_TEST_PING ON )
set(LWS_WITHOUT_TEST_CLIENT ON )
#----
set(LWS_WITH_WOLFSSL ON )
set(LWS_WOLFSSL_INCLUDE_DIRS /tmp/cpp_market${CMAKE_CURRENT_SOURCE_DIR}/.wolfssl_include ) # copy otherwise CMake complains
set(LWS_WOLFSSL_LIBRARIES    ${CMAKE_CURRENT_BINARY_DIR}/extern/wolfssl/libwolfssl.a )
execute_process(COMMAND mkdir -p ${LWS_WOLFSSL_INCLUDE_DIRS})
execute_process(COMMAND
    rsync -avru ${CMAKE_CURRENT_SOURCE_DIR}/extern/wolfssl/wolfssl ${LWS_WOLFSSL_INCLUDE_DIRS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE SSL_HDR_COPY_RES
)
message("@@@@@@@@@@@@@ SSL_HDR_COPY_RES         = ${SSL_HDR_COPY_RES}")
message("@@@@@@@@@@@@@ LWS_WOLFSSL_LIBRARIES    = ${LWS_WOLFSSL_LIBRARIES}")
message("@@@@@@@@@@@@@ LWS_WOLFSSL_INCLUDE_DIRS = ${LWS_WOLFSSL_INCLUDE_DIRS}")
add_subdirectory(extern/libwebsockets)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/extern/libwebsockets/include) # lws_config.h is dynamically generated

#===============================================================================

ADD_DEFINITIONS(-DBUILD_TYPE=\"${CMAKE_BUILD_TYPE}\")

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_USE_RELATIVE_PATHS OFF)

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
    -Wfatal-errors # stop on first error
)

include_directories(SYSTEM .)

add_subdirectory(core)
add_subdirectory(web_socket_simple)
add_subdirectory(web_socket_fast)
add_subdirectory(marketlinks)
