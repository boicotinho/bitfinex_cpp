project("bitfinex_test")

add_executable( ${PROJECT_NAME}
    bitfinex_test.cpp
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC bitfinex)
