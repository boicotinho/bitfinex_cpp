project("bitfinex")

find_package(Boost COMPONENTS) # container/flat_map
find_package(Threads REQUIRED)

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.h)

add_library( ${PROJECT_NAME}
    ${SOURCE_FILES}
)

target_include_directories( ${PROJECT_NAME} SYSTEM INTERFACE ../)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC core Threads::Threads)
