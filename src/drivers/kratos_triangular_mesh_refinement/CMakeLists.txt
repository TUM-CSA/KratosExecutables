set(CMAKE_INCLUDE_CURRENT_DIR ON)

message("**** configuring kratos_triangular_mesh_refinement ****")

add_executable(kratos_triangular_mesh_refinement kratos_triangular_mesh_refinement.cpp)

target_include_directories(
                            kratos_triangular_mesh_refinement PRIVATE
                            "${KRATOS_SOURCE_DIR}/kratos"
                            "${KRATOS_SOURCE_DIR}/external_libraries"
                            "${KRATOS_SOURCE_DIR}/applications/StructuralMechanicsApplication")

target_link_libraries(kratos_triangular_mesh_refinement PRIVATE "${KRATOS_LIBRARY_DIR}/libKratosCore.so" "${KRATOS_LIBRARY_DIR}/libKratosStructuralMechanicsCore.so")

install(TARGETS kratos_triangular_mesh_refinement)