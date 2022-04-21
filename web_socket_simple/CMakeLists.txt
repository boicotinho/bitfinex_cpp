project("web_socket_simple")

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED) # sudo yum install -y openssl-devel

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.h)

add_library( ${PROJECT_NAME}
    ${SOURCE_FILES}
)

target_link_libraries( ${PROJECT_NAME}
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      Threads::Threads
                      OpenSSL::SSL
                      )

# Temp for tls certificate verification lib
# target_include_directories( ${PROJECT_NAME} SYSTEM PRIVATE /home/fabio/dev/certify/include/ ${OPENSSL_INCLUDE_DIR} )
add_subdirectory(utest)
