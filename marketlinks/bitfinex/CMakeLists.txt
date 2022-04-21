project("bitfinex")

# Boost requirements:
#   https://www.boost.org/doc/libs/
#   <boost/json.hpp>            boost v. 1.75   https://www.boost.org/doc/libs/1_78_0/libs/json/doc/html/json/release_notes.html  https://www.boost.org/doc/libs/1_78_0/libs/json/doc/html/index.html
#   <boost/beast>               boost v. 1.66   https://www.boost.org/doc/libs/1_78_0/libs/beast/doc/html/beast/release_notes.html
#   <boost/container/flat_map>  boost v. 1.49?  https://www.boost.org/doc/libs/1_78_0/doc/html/container/release_notes.html  https://www.boost.org/doc/libs/1_78_0/doc/html/container.html
#   <boost/test>                boost v. 1.21   https://www.boost.org/doc/libs/1_78_0/libs/test/doc/html/index.html
#   Redhat 7 usually have 1.53, but boost/beast is an important lib here.
find_package(Boost 1.66 COMPONENTS)

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.h)

add_library( ${PROJECT_NAME}
    ${SOURCE_FILES}
)

target_include_directories( ${PROJECT_NAME} SYSTEM INTERFACE ../../)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    core web_socket_simple marketlinks_common)

add_subdirectory(utest)
