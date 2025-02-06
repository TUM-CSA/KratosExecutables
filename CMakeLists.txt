project(BenchmarkApplication CXX)

find_package(benchmark REQUIRED)

# Generate sources from templates
file(GLOB source_templates "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp.in")

foreach(source_template ${source_templates})
    # Configure and output to intermediate file (read-write permissions)
    get_filename_component(source_template_name "${source_template}" NAME_WE)
    configure_file("${source_template}" "${CMAKE_CURRENT_SOURCE_DIR}/src/${source_template_name}.cpp")

    # Withdraw write permission from the generated file
    file(CHMOD
         "${CMAKE_CURRENT_SOURCE_DIR}/src/${source_template_name}.cpp"
         FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endforeach()

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/*.cpp)
file(GLOB drivers ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/*.cpp)

foreach(driver ${drivers})
    get_filename_component(driver_name "${driver}" NAME_WE)
    add_executable(${driver_name} ${sources} ${driver})
    target_link_libraries(${driver_name} PRIVATE
                          KratosCore
                          benchmark::benchmark)
    target_include_directories(${driver_name} PRIVATE
                               "${KRATOS_SOURCE_DIR}/kratos"
                               "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
endforeach()
