#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

# Temp, Hum lib
add_subdirectory(MT3620_Grove_Shield_Library)

cmake_minimum_required (VERSION 3.8)
project (SphereTest_HighLevelCore C)

# Create executable
add_executable (${PROJECT_NAME} main.c)
target_link_libraries (${PROJECT_NAME} applibs pthread gcc_s c MT3620_Grove_Shield_Library)

# Add MakeImage post-build command
include (${AZURE_SPHERE_MAKE_IMAGE_FILE})
