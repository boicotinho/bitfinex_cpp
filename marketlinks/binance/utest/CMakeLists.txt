project("binance_utest")

add_executable( ${PROJECT_NAME}
    ut_main.cpp
    ut_poc1.cpp
    ut_parser.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC binance )
