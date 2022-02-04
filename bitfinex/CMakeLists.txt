project("bitfinex")

add_library( ${PROJECT_NAME}
    bitfinex.cpp
)

target_include_directories( ${PROJECT_NAME} SYSTEM INTERFACE ../)
