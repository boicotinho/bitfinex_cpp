project("bitfinex")

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.h)

add_library( ${PROJECT_NAME}
    ${SOURCE_FILES}
)

target_include_directories( ${PROJECT_NAME} SYSTEM INTERFACE ../)
