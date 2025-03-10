set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(benchmark REQUIRED)

message("**** configuring kratos_kd_tree_benchmark ****")

add_executable(kratos_kd_tree_benchmark kratos_kd_tree_benchmark.cpp)

target_include_directories(
                            kratos_kd_tree_benchmark PRIVATE
                            "${KRATOS_SOURCE_DIR}/kratos"
                            "${KRATOS_SOURCE_DIR}/external_libraries"
                            "${CMAKE_CURRENT_SOURCE_DIR}/external_includes")

target_link_libraries(kratos_kd_tree_benchmark PRIVATE "${KRATOS_LIBRARY_DIR}/libKratosCore.so" benchmark::benchmark)

install(TARGETS kratos_kd_tree_benchmark)