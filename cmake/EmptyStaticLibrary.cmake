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

#
# EmptyStaticLibrary: a module for add static libraries without source files
# ------------------------------------------------------------------------------

if(ESL_C_SOURCE_FILE)
    return()
endif()

set(ESL_C_SOURCE_FILE "${CMAKE_BINARY_DIR}/esl_empty_src.c")

add_custom_command(OUTPUT "${ESL_C_SOURCE_FILE}"
    COMMAND "${CMAKE_COMMAND}" "-E" "touch" "${ESL_C_SOURCE_FILE}")

function(add_empty_static_library NAME)
    add_library("${NAME}" STATIC "${ESL_C_SOURCE_FILE}")
endfunction()
