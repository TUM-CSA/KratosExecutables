set(CMAKE_INCLUDE_CURRENT_DIR ON)

message("**** configuring kratos_hello_world ****")

add_executable(kratos_hello_world kratos_hello_world.cpp)

target_include_directories(
                            kratos_hello_world PRIVATE
                            "${KRATOS_SOURCE_DIR}/kratos"
                            "${KRATOS_SOURCE_DIR}/external_libraries")

target_link_libraries(kratos_hello_world PRIVATE "${KRATOS_LIBRARY_DIR}/libKratosCore.so")
set_target_properties(kratos_hello_world PROPERTIES INSTALL_RPATH "${KRATOS_LIBRARY_DIR}")

install(TARGETS kratos_hello_world)