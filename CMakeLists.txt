# Copyright 2021 Jean Pierre Cimalando
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.14)
project(ysfx VERSION "0.0.0" LANGUAGES C CXX ASM)

# check if we are building from this project, or are imported by another
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(YSFX_BUILD_FROM_HERE TRUE)
else()
    set(YSFX_BUILD_FROM_HERE FALSE)
endif()

option(YSFX_PORTABLE "Disable architecture-dependent code" OFF)
option(YSFX_GFX "Build graphics support" ON)
option(YSFX_PLUGIN "Build audio plugin" "${YSFX_BUILD_FROM_HERE}")
option(YSFX_PLUGIN_LTO "Enable link-time optimization for plugin" OFF)
option(YSFX_PLUGIN_FORCE_DEBUG "Build debug features in plugin" OFF)
option(YSFX_PLUGIN_COPY "Copy the plugin after build" ON)
set(YSFX_PLUGIN_JUCE_SDK_PATH "" CACHE PATH "Custom path to the JUCE SDK")
set(YSFX_PLUGIN_VST3_SDK_PATH "" CACHE PATH "Custom path to the VST3 SDK")
option(YSFX_TESTS "Build unit tests" OFF)
option(YSFX_TOOLS "Build tools" OFF)
option(YSFX_SKIP_CHECKSUM "Skip the checksum" OFF)
option(YSFX_DEEP_STRIP "Strip the library clean of non-API symbols (if platform supports)" OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(BuildType)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_ASM_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT MSVC)
    add_compile_options(
        "-Wall"
        "-Werror=return-type")
endif()
if(MSVC)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif()

###
include("cmake.deps.txt")
include("cmake.wdl.txt")
include("cmake.ysfx.txt")
if(YSFX_TESTS)
    include("cmake.tests.txt")
endif()
if(YSFX_PLUGIN)
    include("cmake.plugin.txt")
endif()
if(YSFX_TOOLS)
    include("cmake.tools.txt")
endif()
