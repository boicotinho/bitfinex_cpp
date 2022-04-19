project("test_lws")

find_package(Threads REQUIRED)

add_executable( ${PROJECT_NAME}
    test_lws_main.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    core web_socket wolfssl websockets Threads::Threads)
