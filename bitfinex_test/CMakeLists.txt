project("bitfinex_test")

add_executable( ${PROJECT_NAME}
    ut_main.cpp
    ut_book_map.cpp
    ut_level_book.cpp
    ut_dense_map.cpp
    ut_json_obj.cpp
    ut_parser.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC bitfinex)
