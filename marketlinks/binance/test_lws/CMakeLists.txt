project("test_lws")

add_executable( ${PROJECT_NAME}
    test_lws_main.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    core websockets)
