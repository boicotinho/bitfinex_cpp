project("marketlinks_common")

#file (GLOB SOURCE_FILES *.cpp)
#file (GLOB HEADER_FILES *.h)

add_library( ${PROJECT_NAME}
    json_obj.cpp
)

target_include_directories( ${PROJECT_NAME} SYSTEM INTERFACE ../../)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    core
    web_socket_fast
    )
