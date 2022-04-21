project("web_socket_fast_utest")

add_executable( ${PROJECT_NAME}
    ut_main.cpp
    ut_test1.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    web_socket_async
)
