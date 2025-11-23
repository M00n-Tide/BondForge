cmake_minimum_required(VERSION 3.16)

project(BondForge VERSION 2.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts WebEngineCore WebEngineWidgets)

# Find optional packages
find_package(RDKit QUIET)
set(RDKIT_FOUND ${RDKit_FOUND})

find_package(mlpack QUIET)
set(MLPACK_FOUND ${MLPACK_FOUND})

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_SOURCE_DIR}/ui)
include_directories(${CMAKE_SOURCE_DIR}/services)
include_directories(${CMAKE_SOURCE_DIR}/utils)

# Add subdirectories
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(services)
add_subdirectory(utils)

# Main executable
set(SOURCES
    main.cpp
    # Add other source files here
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Charts 
    Qt6::WebEngineCore 
    Qt6::WebEngineWidgets
    bondforge_core
    bondforge_ui
    bondforge_services
    bondforge_utils
)

# Optional dependencies
if(RDKIT_FOUND)
    target_link_libraries(${PROJECT_NAME} ${RDKit_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RDKit_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE RDKIT_ENABLED)
endif()

if(MLPACK_FOUND)
    target_link_libraries(${PROJECT_NAME} mlpack)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MLPACK_ENABLED)
endif()

# Platform specific settings
if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES config/bondforge.json DESTINATION config)
install(DIRECTORY config/i18n DESTINATION config)
install(DIRECTORY plugins DESTINATION .)
install(DIRECTORY examples DESTINATION .)