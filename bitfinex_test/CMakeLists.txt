project("bitfinex_test")

add_executable( ${PROJECT_NAME}
    ut_main.cpp
    ut_book_map.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC bitfinex)
