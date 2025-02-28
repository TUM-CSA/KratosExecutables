set(CMAKE_INCLUDE_CURRENT_DIR ON)

message("**** configuring kratos_mdpa_visualization ****")

add_executable(kratos_mdpa_visualization kratos_mdpa_visualization.cpp)

target_include_directories(
                            kratos_mdpa_visualization PRIVATE
                            "${KRATOS_SOURCE_DIR}/kratos"
                            "${KRATOS_SOURCE_DIR}/external_libraries"
                            "${KRATOS_SOURCE_DIR}/applications/StructuralMechanicsApplication")

target_link_libraries(kratos_mdpa_visualization PRIVATE "${KRATOS_LIBRARY_DIR}/libKratosCore.so" "${KRATOS_LIBRARY_DIR}/libKratosStructuralMechanicsCore.so")

install(TARGETS kratos_mdpa_visualization)