project("binance")

add_library( ${PROJECT_NAME}
    market_data_feed.cpp
)

target_include_directories( ${PROJECT_NAME} SYSTEM INTERFACE ../../)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    core marketlinks_common )

add_subdirectory(utest)
add_subdirectory(test_poc_sarb)
add_subdirectory(test_lws)
