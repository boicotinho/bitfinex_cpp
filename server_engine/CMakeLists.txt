project("server_engine")

add_library( ${PROJECT_NAME}
    server_engine.cpp
)

target_link_libraries( ${PROJECT_NAME}
    core web_socket_async marketlinks
    )

#add_subdirectory(utest)
