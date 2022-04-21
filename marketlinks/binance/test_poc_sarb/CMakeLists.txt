project("test_poc_sarb")

add_executable( ${PROJECT_NAME}
    test_poc_sarb.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    core web_socket_simple )
