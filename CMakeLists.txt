cmake_minimum_required(VERSION 3.20)

project(KratosExecutables
        LANGUAGES CXX
        VERSION 0.1.0)

string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

# Fetch the kratos source directory.
if(NOT DEFINED KRATOS_SOURCE_DIR)
    message(FATAL_ERROR "KRATOS_SOURCE_DIR is not defined. Set it to Kratos' source directory.")
elseif(${KRATOS_SOURCE_DIR} STREQUAL "")
    message(FATAL_ERROR "KRATOS_SOURCE_DIR is empty. Set it to Kratos' source directory.")
elseif(NOT IS_DIRECTORY ${KRATOS_SOURCE_DIR})
    message(FATAL_ERROR "The Kratos source directory ${KRATOS_SOURCE_DIR} does not exist or is not a directory.")
else()
    message(STATUS "Fetch Kratos source files from ${KRATOS_SOURCE_DIR}.")
endif()

# Fetch the kratos library installation directory.
if(NOT DEFINED KRATOS_LIBRARY_DIR)
    message(FATAL_ERROR "KRATOS_LIBRARY_DIR is not defined. Set it to the directory where Kratos libraries are installed.")
elseif(${KRATOS_LIBRARY_DIR} STREQUAL "")
    message(FATAL_ERROR "KRATOS_LIBRARY_DIR cmake variable is empty. Please set it to Kratos library dir.")
elseif(NOT IS_DIRECTORY ${KRATOS_LIBRARY_DIR})
    message(FATAL_ERROR "The Kratos library directory ${KRATOS_LIBRARY_DIR} not found.")
else()
    message(STATUS "Linking Kratos libraries files from ${KRATOS_LIBRARY_DIR}...")
endif()

# Collect all compiled Kratos applications.
# On linux, assume all of them begin with "libKratos" and end with "Core" and then a suffix.
set(${PROJECT_NAME}_compile_definitions "")
set(${PROJECT_NAME_UPPER}_LIBRARY_PREFIX "libKratos")
set(${PROJECT_NAME_UPPER}_LIBRARY_SUFFIX "Core.so")
set(${PROJECT_NAME}_include "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
set(${PROJECT_NAME}_link_libraries "")

file(GLOB kratos_libraries "${KRATOS_LIBRARY_DIR}/${${PROJECT_NAME_UPPER}}*${${PROJECT_NAME_UPPER}_LIBRARY_SUFFIX}")
foreach(kratos_library ${kratos_libraries})
    get_filename_component(kratos_library_name "${kratos_library}" NAME_WE)
    string(REGEX MATCH "libKratos([a-zA-Z0-9]*)Core.*" _ "${kratos_library_name}")

    if (NOT ${CMAKE_MATCH_0} STREQUAL "")
        set(kratos_application_name "${CMAKE_MATCH_1}")

        if(DEFINED ${kratos_application_name}Application_INCLUDE)
            list(APPEND ${PROJECT_NAME}_include "${${kratos_application_name}Application_INCLUDE}")
        elseif("${kratos_application_name}" STREQUAL "")
            list(APPEND ${PROJECT_NAME}_include "${KRATOS_SOURCE_DIR}/kratos" "${KRATOS_SOURCE_DIR}/external_libraries")
        else()
            list(APPEND ${PROJECT_NAME}_include "${KRATOS_SOURCE_DIR}/applications/${kratos_application_name}Application")
        endif()

        list(APPEND ${PROJECT_NAME}_link_libraries "${kratos_library}")
        set(${PROJECT_NAME_UPPER}_${kratos_application_name}Application 1)
        list(APPEND ${PROJECT_NAME}_compile_definitions "${PROJECT_NAME_UPPER}_${kratos_application_name}")
        message(STATUS "Found ${kratos_application_name} (${kratos_library_name}).")
    endif()
endforeach()

# Generate sources from templates.
file(GLOB source_templates "${CMAKE_CURRENT_SOURCE_DIR}/src/impl/*.cpp.in")
foreach(source_template ${source_templates})
    # Configure and generate the source file.
    get_filename_component(source_template_name "${source_template}" NAME_WE)
    configure_file("${source_template}" "${CMAKE_CURRENT_SOURCE_DIR}/src/impl/${source_template_name}.cpp")

    # Withdraw write permissions from the generated file.
    file(CHMOD
         "${CMAKE_CURRENT_SOURCE_DIR}/src/impl/${source_template_name}.cpp"
         FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endforeach()

# Collect common sources.
file(GLOB ${PROJECT_NAME}_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/*.cpp)

# Configure drivers.
file(GLOB drivers LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/*)
foreach(driver ${drivers})
    if (IS_DIRECTORY "${driver}")
        add_subdirectory(${driver})
    endif()
endforeach()
