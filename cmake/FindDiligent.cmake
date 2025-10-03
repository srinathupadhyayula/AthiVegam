# FindDiligent.cmake
# Finds the Diligent Engine library
#
# This module defines:
#  Diligent_FOUND - System has Diligent Engine
#  Diligent_INCLUDE_DIRS - The Diligent Engine include directories
#  Diligent_LIBRARIES - The libraries needed to use Diligent Engine
#
# Usage:
#  find_package(Diligent REQUIRED)
#  target_link_libraries(MyTarget PRIVATE Diligent::Core Diligent::GraphicsEngineD3D12)

# Look for Diligent Engine in external directory
find_path(Diligent_ROOT_DIR
    NAMES DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/external/DiligentEngine
        ${CMAKE_SOURCE_DIR}/external/DiligentEngine
    NO_DEFAULT_PATH
)

if(Diligent_ROOT_DIR)
    set(Diligent_FOUND TRUE)
    set(Diligent_INCLUDE_DIRS
        ${Diligent_ROOT_DIR}/DiligentCore
        ${Diligent_ROOT_DIR}/DiligentCore/Graphics/GraphicsEngine/interface
        ${Diligent_ROOT_DIR}/DiligentCore/Graphics/GraphicsEngineD3D12/interface
        ${Diligent_ROOT_DIR}/DiligentCore/Graphics/GraphicsEngineVulkan/interface
        ${Diligent_ROOT_DIR}/DiligentCore/Common/interface
        ${Diligent_ROOT_DIR}/DiligentCore/Platforms/interface
    )
    
    message(STATUS "Found Diligent Engine: ${Diligent_ROOT_DIR}")
else()
    set(Diligent_FOUND FALSE)
    if(Diligent_FIND_REQUIRED)
        message(FATAL_ERROR "Diligent Engine not found. Please run: git submodule update --init --recursive")
    endif()
endif()

# Create imported targets (will be populated when Diligent is actually built)
if(Diligent_FOUND AND NOT TARGET Diligent::Core)
    add_library(Diligent::Core INTERFACE IMPORTED)
    set_target_properties(Diligent::Core PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Diligent_INCLUDE_DIRS}"
    )
endif()

