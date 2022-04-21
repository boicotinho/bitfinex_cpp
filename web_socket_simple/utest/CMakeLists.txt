project("web_socket_test")

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED) # sudo yum install -y openssl-devel

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.h)

add_executable( ${PROJECT_NAME}
    #${SOURCE_FILES}
    ut_main.cpp
    ut_url.cpp
    ut_client_recv.cpp
    ut_synchronous_hitbtc.cpp
    ut_synchronous_bitfinex.cpp
    #test_async_echo.cpp # prototype app with its own main() and command line
)
target_link_libraries( ${PROJECT_NAME}
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      Threads::Threads
                      OpenSSL::SSL
                      web_socket_simple
                      )
# Temp for tls certificate verification lib
# target_include_directories( ${PROJECT_NAME} SYSTEM PRIVATE /home/fabio/dev/certify/include/ ${OPENSSL_INCLUDE_DIR} )
