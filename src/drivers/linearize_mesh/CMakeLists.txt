set(PARENT_PROJECT_NAME ${PROJECT_NAME})
project(linearize_mesh)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
               "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.cpp"
               ${${PARENT_PROJECT_NAME}_sources})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${${PARENT_PROJECT_NAME}_compile_definitions})
target_include_directories(${PROJECT_NAME} PRIVATE ${${PARENT_PROJECT_NAME}_include})
target_link_libraries(${PROJECT_NAME} PRIVATE ${${PARENT_PROJECT_NAME}_link_libraries})
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${KRATOS_LIBRARY_DIR}")

install(TARGETS ${PROJECT_NAME})
