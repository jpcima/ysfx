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

if(NOT SOURCE_FILE OR NOT CHECKSUM_FILE)
    message(FATAL_ERROR "Required arguments are missing")
endif()

if(NOT TEXT_MODE)
    file(SHA512 "${SOURCE_FILE}" CHECKSUM_OBTAINED)
else()
    file(READ "${SOURCE_FILE}" SOURCE_DATA)
    string(REPLACE "\r\n" "\n" SOURCE_DATA "${SOURCE_DATA}")
    string(SHA512 CHECKSUM_OBTAINED "${SOURCE_DATA}")
endif()

file(READ "${CHECKSUM_FILE}" CHECKSUM_EXPECTED)
string(STRIP "${CHECKSUM_EXPECTED}" CHECKSUM_EXPECTED)

get_filename_component(SOURCE_FILE_BASE "${SOURCE_FILE}" NAME)

if(CHECKSUM_OBTAINED STREQUAL CHECKSUM_EXPECTED)
    message(STATUS "Checksum valid: ${SOURCE_FILE_BASE}")
else()
    message(FATAL_ERROR "Checksum invalid: ${SOURCE_FILE_BASE} ('${CHECKSUM_OBTAINED}' != '${CHECKSUM_EXPECTED}')")
endif()
