cmake_minimum_required(VERSION 3.20)
project(KratosExecutables CXX)

# fetching the kratos source dir
if(NOT DEFINED KRATOS_SOURCE_DIR)
    message(FATAL_ERROR "KRATOS_SOURCE_DIR cmake variable is not defined. Please set it to Kratos source root.")
elseif(${KRATOS_SOURCE_DIR} STREQUAL "")
    message(FATAL_ERROR "KRATOS_SOURCE_DIR cmake variable is empty. Please set it to Kratos source root.")
elseif(NOT IS_DIRECTORY ${KRATOS_SOURCE_DIR})
    message(FATAL_ERROR "The Kratos source directory ${KRATOS_SOURCE_DIR} not found.")
else()
    message(STATUS "Fetching Kratos source files from ${KRATOS_SOURCE_DIR}...")
endif()

# fetching the kratos library installation dir
if(NOT DEFINED KRATOS_LIBRARY_DIR)
    message(FATAL_ERROR "KRATOS_LIBRARY_DIR cmake variable is not defined. Please set it to Kratos library dir.")
elseif(${KRATOS_LIBRARY_DIR} STREQUAL "")
    message(FATAL_ERROR "KRATOS_LIBRARY_DIR cmake variable is empty. Please set it to Kratos library dir.")
elseif(NOT IS_DIRECTORY ${KRATOS_LIBRARY_DIR})
    message(FATAL_ERROR "The Kratos library directory ${KRATOS_LIBRARY_DIR} not found.")
else()
    message(STATUS "Linking Kratos libraries files from ${KRATOS_LIBRARY_DIR}...")
endif()

# Generate sources from templates
file(GLOB source_templates "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp.in")

set(EXECUTABLES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

foreach(source_template ${source_templates})
    # Configure and output to intermediate file (read-write permissions)
    get_filename_component(source_template_name "${source_template}" NAME_WE)
    configure_file("${source_template}" "${CMAKE_CURRENT_SOURCE_DIR}/src/${source_template_name}.cpp")

    # Withdraw write permission from the generated file
    file(CHMOD
         "${CMAKE_CURRENT_SOURCE_DIR}/src/${source_template_name}.cpp"
         FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endforeach()

# file(GLOB drivers ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/*.cpp)
file(GLOB drivers LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/*)

foreach(driver ${drivers})
    add_subdirectory(${driver})
endforeach()
