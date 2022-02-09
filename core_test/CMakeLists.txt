project("core_test")

find_package(Threads REQUIRED)

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.h)

add_executable( ${PROJECT_NAME}
    ${SOURCE_FILES}
)

target_link_libraries( ${PROJECT_NAME}
                      Threads::Threads
                      core
                      )
