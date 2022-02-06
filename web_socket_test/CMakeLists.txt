project("web_socket_test")

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.h)

add_executable( ${PROJECT_NAME}
    #${SOURCE_FILES}
    ut_basics.cpp
)
target_link_libraries( ${PROJECT_NAME}
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      )

