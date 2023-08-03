cmake_minimum_required(VERSION 3.10)

project(WhereIsMyBus CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

set(PROTO_FILES
transport_catalogue.proto
map_renderer.proto
svg.proto
graph.proto
transport_router.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

set(BUSESBASE_FILES
main.cpp
domain.h
domain.cpp
geo.h
graph.h
geo.cpp
json.h
json.cpp
json_builder.h
json_builder.cpp
json_reader.h
json_reader.cpp
map_renderer.h
map_renderer.cpp
ranges.h
router.h
transport_router.h
transport_router.cpp
serialization.h
serialization.cpp
svg.h
svg.cpp
request_handler.h
request_handler.cpp
transport_catalogue.h
transport_catalogue.cpp
${PROTO_FILES}
)

add_executable(wimbus ${PROTO_SRCS} ${PROTO_HDRS} ${BUSESBASE_FILES})
target_include_directories(wimbus PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(wimbus PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

#target_link_libraries(make_base "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
target_link_libraries(wimbus ${Protobuf_LIBRARY} Threads::Threads)