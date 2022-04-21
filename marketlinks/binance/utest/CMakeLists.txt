project("binance_utest")

#find_package(Threads REQUIRED)

add_executable( ${PROJECT_NAME}
    ut_main.cpp
    ut_poc1.cpp
    ut_parser.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    #core web_socket wolfssl websockets Threads::Threads
    marketlinks_common
    )
