set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(benchmark REQUIRED)

message("**** configuring kratos_distance_benchmark ****")

file(GLOB sources ${EXECUTABLES_SOURCE_DIR}/src/impl/*.cpp)

add_executable(kratos_distance_benchmark ${sources} distance_benchmark.cpp)

target_include_directories(kratos_distance_benchmark PRIVATE
                            "${KRATOS_SOURCE_DIR}/kratos"
                            "${KRATOS_SOURCE_DIR}/external_libraries"
                            "${EXECUTABLES_SOURCE_DIR}/src/include")

target_link_libraries(kratos_distance_benchmark PRIVATE
                        "${KRATOS_LIBRARY_DIR}/libKratosCore.so"
                        benchmark::benchmark)

install(TARGETS kratos_distance_benchmark)