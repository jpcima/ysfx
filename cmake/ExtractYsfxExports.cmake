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

################################################################################
# This tool extracts symbol names from the header `ysfx.h`.

macro(extract_next_line text_var line_var)
    string(FIND "${${text_var}}" "\n" _pos)
    if(NOT _pos EQUAL -1)
        string(SUBSTRING "${${text_var}}" 0 "${_pos}" "${line_var}")
        math(EXPR _pos "${_pos}+1")
        string(SUBSTRING "${${text_var}}" "${_pos}" -1 "${text_var}")
    else()
        set("${line_var}" "${${text_var}}")
        set("${text_var}" "")
    endif()
    unset(_pos)
endmacro()

function(extract_api_names file_path result_var)
    set(RESULT)
    file(READ "${file_path}" FILE_TEXT)
    while(NOT FILE_TEXT STREQUAL "")
        extract_next_line(FILE_TEXT LINE)
        string(REGEX MATCH "^[ \\t]*YSFX_API[ \\t]+" PART "${LINE}")
        if(NOT PART STREQUAL "")
            string(REGEX MATCH "(ysfx_[a-zA-Z0-9_]+)[ \\t]*\\(" PART "${LINE}")
            if(NOT PART STREQUAL "")
                string(LENGTH "${PART}" LENGTH)
                math(EXPR LENGTH "${LENGTH}-1")
                string(SUBSTRING "${PART}" 0 "${LENGTH}" PART)
                list(APPEND RESULT "${PART}")
            endif()
        endif()
    endwhile()
    set("${result_var}" "${RESULT}" PARENT_SCOPE)
endfunction()

if(NOT EXISTS "include/ysfx.h")
    message(FATAL_ERROR "This script must run from the top directory.")
endif()

extract_api_names("include/ysfx.h" API_NAMES)
file(MAKE_DIRECTORY "api")

file(WRITE "exports/ysfx.txt" "")
foreach(_name IN LISTS API_NAMES)
    file(APPEND "exports/ysfx.txt" "${_name}\n")
endforeach()
