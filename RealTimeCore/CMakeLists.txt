#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required (VERSION 3.8)
project (BlueControl_RealTimeCore C)

# Create executable
add_executable (${PROJECT_NAME} main.c)
target_link_libraries (${PROJECT_NAME})
set_target_properties (${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)

# Add MakeImage post-build command
include ("${AZURE_SPHERE_MAKE_IMAGE_FILE}")
