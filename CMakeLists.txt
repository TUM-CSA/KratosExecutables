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

# Check the availability of Kratos applications.
set(${PROJECT_NAME}_compile_definitions "")
set(${PROJECT_NAME}_include "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
set(${PROJECT_NAME}_link_libraries "")

if (IS_DIRECTORY "${KRATOS_SOURCE_DIR}/kratos" AND EXISTS "${KRATOS_LIBRARY_DIR}/libKratosCore.so")
    message(STATUS "Found KratosCore.")
    list(APPEND ${PROJECT_NAME}_compile_definitions "${PROJECT_NAME_UPPER}_KRATOS_CORE")
    list(APPEND ${PROJECT_NAME}_include "${KRATOS_SOURCE_DIR}/kratos" "${KRATOS_SOURCE_DIR}/external_libraries")
    list(APPEND ${PROJECT_NAME}_link_libraries "${KRATOS_LIBRARY_DIR}/libKratosCore.so")
else()
    message(FATAL_ERROR "Could not find the Kratos core library.")
endif()

if (IS_DIRECTORY "${KRATOS_SOURCE_DIR}/applications/HDF5Application" AND EXISTS "${KRATOS_LIBRARY_DIR}/libKratosHDF5Core.so")
    message(STATUS "Found HDF5Application.")
    list(APPEND ${PROJECT_NAME}_compile_definitions "${PROJECT_NAME_UPPER}_KRATOS_HDF5_APPLICATION")
    list(APPEND ${PROJECT_NAME}_include "${KRATOS_SOURCE_DIR}/applications/HDF5Application")
    list(APPEND ${PROJECT_NAME}_link_libraries "${KRATOS_LIBRARY_DIR}/libKratosHDF5Core.so")
else()
    message(STATUS "Could not find the Kratos HDF5 library.")
endif()

if (IS_DIRECTORY "${KRATOS_SOURCE_DIR}/applications/MedApplication" AND EXISTS "${KRATOS_LIBRARY_DIR}/libKratosMedCore.so")
    message(STATUS "Found MedApplication.")
    list(APPEND ${PROJECT_NAME}_compile_definitions "${PROJECT_NAME_UPPER}_KRATOS_Med_APPLICATION")
    list(APPEND ${PROJECT_NAME}_include "${KRATOS_SOURCE_DIR}/applications/MedApplication")
    list(APPEND ${PROJECT_NAME}_link_libraries "${KRATOS_LIBRARY_DIR}/libKratosMedCore.so")
else()
    message(STATUS "Could not find the Kratos Med library.")
endif()

# Collect all compiled Kratos applications.
# On linux, assume all of them begin with "libKratos" and end with "Core" and then a suffix.
set(${PROJECT_NAME_UPPER}_LIBRARY_PREFIX "libKratos")
set(${PROJECT_NAME_UPPER}_LIBRARY_SUFFIX "Core.so")

#file(GLOB kratos_libraries "${KRATOS_LIBRARY_DIR}/${${PROJECT_NAME_UPPER}}*${${PROJECT_NAME_UPPER}_LIBRARY_SUFFIX}")
#foreach(kratos_library ${kratos_libraries})
#    get_filename_component(kratos_library_name "${kratos_library}" NAME_WE)
#    string(REGEX MATCH "libKratos([a-zA-Z]+)Core.*" _ "${kratos_library_name}")
#    if (NOT ${CMAKE_MATCH_0} STREQUAL "")
#        # It gets a bit ugly here.
#        set(kratos_application_name "${CMAKE_MATCH_1}")
#        message(STATUS "Found ${kratos_application_name} (${kratos_library_name}).")
#    endif()
#endforeach()

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

if(IS_DIRECTORY "${KRATOS_SOURCE_DIR}/applications/HDF5Application" AND EXISTS "${KRATOS_LIBRARY_DIR}/libKratosHDF5Core.so")
    message(STATUS "Found KratosHDF5Application.")
    list(APPEND ${PROJECT_NAME}_compile_definitions "${PROJECT_NAME_UPPER}_HDF5_APPLICATION")
    list(APPEND ${PROJECT_NAME}_include "${KRATOS_SOURCE_DIR}/applications/HDF5Application")
    list(APPEND ${PROJECT_NAME}_link_libraries "${KRATOS_LIBRARY_DIR}/libKratosHDF5Core.so")
else()
    message(STATUS "Could not find KratosHDF5Application.")
endif()

if(IS_DIRECTORY "${KRATOS_SOURCE_DIR}/applications/MedApplication" AND EXISTS "${KRATOS_LIBRARY_DIR}/libKratosMedCore.so")
    message(STATUS "Found KratosMedApplication.")
    list(APPEND ${PROJECT_NAME}_compile_definitions "${PROJECT_NAME_UPPER}_MED_APPLICATION")
    list(APPEND ${PROJECT_NAME}_include "${KRATOS_SOURCE_DIR}/applications/MedApplication")
    list(APPEND ${PROJECT_NAME}_link_libraries "${KRATOS_LIBRARY_DIR}/libKratosMedCore.so")
else()
    message(STATUS "Could not find KratosMedApplication.")
endif()

# Collect common sources.
file(GLOB ${PROJECT_NAME}_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/*.cpp)

# Configure drivers.
file(GLOB drivers LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/*)
foreach(driver ${drivers})
    if (IS_DIRECTORY "${driver}")
        add_subdirectory(${driver})
    endif()
endforeach()
