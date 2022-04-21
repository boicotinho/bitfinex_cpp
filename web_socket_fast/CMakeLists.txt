project("web_socket_fast")

add_library( ${PROJECT_NAME}
    ws_thread_context.cpp
    ws_client.cpp
)

target_link_libraries( ${PROJECT_NAME}
    core
    websockets # libwebsockets compiled with wolfssl
    )

add_subdirectory(utest)
