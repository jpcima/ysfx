/* -*- mode: c++; -*- */
// Copyright 2021 Jean Pierre Cimalando
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
//

#include "ysfx_parse.hpp"
#include "ysfx_utils.hpp"
#include <string>

bool ysfx_parse_filename(const char *line, ysfx_parsed_filename_t &filename)
{
    filename = ysfx_parsed_filename_t{};

    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:flags:tags = 1;
    re2c:define:YYCTYPE = char;
    */

    const char *YYCURSOR;
    const char *YYMARKER;

    /*!stags:re2c format = "const char* @@; "; */

    //NOTE(jpc): parser intensionally very permissive

    /*!re2c
    num = ([a-zA-Z0-9]|"."|"+"|"-")+;
    sp = [ \f\n\r\t\v];
    */

    YYCURSOR = line;

    //--------------------------------------------------------------------------
    // Parse Filename

    {
        const char *idx1, *idx2, *path1;

        /*!re2c
        "filename:" @idx1 num @idx2 "," @path1 [^\x00]*
        {
             int64_t index = (int64_t)ysfx::dot_atof(std::string(idx1, idx2).c_str());
             if (index < 0 || index > ~(uint32_t)0)
                 return false;
             filename.index = (uint32_t)index;
             filename.filename = ysfx::trim(path1, &ysfx::ascii_isspace);
             goto end_filename;
        }
        *
        {
            return false;
        }
        */

        end_filename: ;
    }

    return true;
}
