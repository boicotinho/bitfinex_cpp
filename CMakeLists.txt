cmake_minimum_required(VERSION 2.8.12)
project(bitfinex_cpp)

ADD_DEFINITIONS(-DBUILD_TYPE=\"${CMAKE_BUILD_TYPE}\")

set(CMAKE_CXX_STANDARD 11)

add_compile_options(
    -Wall -Wextra -Wpedantic -Werror -march=native -mtune=native
    -Wdeprecated -W -Wpointer-arith -Wno-uninitialized -Wno-unused
    -Wno-sign-compare -Wno-comment -Wno-unknown-pragmas -Wno-unused-parameter
    -Wno-invalid-offsetof -no-integrated-cpp -fno-strict-aliasing
    -Werror -Werror=return-type
)

include_directories(SYSTEM .)

add_subdirectory(core)
add_subdirectory(bitfinex)
add_subdirectory(bitfinex_test)

add_subdirectory(web_socket_test)
