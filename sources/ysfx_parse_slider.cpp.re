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
#include <cstddef>
#include <cstdlib>

bool ysfx_parse_slider(const char *line, ysfx_slider_t &slider)
{
    slider = ysfx_slider_t{};

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
    // Parse ID and Var

    {
        const char *id1, *id2, *var1, *var2;

        /*!re2c
        "slider" @id1 [1-9][0-9]* @id2 ":"
        (@var1 [^=\x00]+ @var2 "=")?
             //NOTE(jpc) ref impl allows var to have any chars including spaces
             //          it's useless, but do anyway
        {
            slider.id = (uint32_t)(atoi(std::string(id1, id2).c_str()) - 1);
            if (var1)
                slider.var = std::string(var1, var2);
            else
                slider.var = std::string(line, id2);
            goto end_id;
        }
        *
        {
            return false;
        }
        */

        end_id: ;
    }

    if (*YYCURSOR != '/')
    {
        //--------------------------------------------------------------------------
        // Parse Range segment

        const char *def1, *def2, *min1, *min2, *max1, *max2, *inc1, *inc2, *nam1, *nam2;

        /*!re2c
        sp* @def1 num? @def2
        sp* (","|("<"
          sp* (@min1 num? @min2 sp* ("," sp* @max1 num? @max2 sp* ("," sp* @inc1 num? @inc2 sp*)?)?)?
          sp* (">"|([^{\x00]* "{" @nam1 [^}\x00]* @nam2 "}"? sp* ">")) sp* ","?
        ))
        {
            slider.def = ysfx::dot_atof(std::string(def1, def2).c_str());
            if (min1)
                slider.min = ysfx::dot_atof(std::string(min1, min2).c_str());
            if (max1)
                slider.max = ysfx::dot_atof(std::string(max1, max2).c_str());
            if (inc1)
                slider.inc = ysfx::dot_atof(std::string(inc1, inc2).c_str());
            if (nam1) {
                slider.is_enum = true;
                slider.enum_names = ysfx::split_strings_noempty(
                    std::string(nam1, nam2).c_str(),
                   [](char c) -> bool { return c == ','; });
                for (std::string &name : slider.enum_names)
                    name = ysfx::trim(name.c_str(), &ysfx::ascii_isspace);
            }
            goto end_range;
        }
        *
        {
            return false;
        }
        */
    }
    else
    {
        //--------------------------------------------------------------------------
        // Parse Path segment

        const char *path1, *path2, *def1, *def2;

        /*!re2c
        @path1 "/" [^:\x00]* @path2 ":" sp* @def1 num? @def2 sp* ":"
        {
            slider.path = std::string(path1, path2);
            slider.def = ysfx::dot_atof(std::string(def1, def2).c_str());
            slider.inc = 1;
            slider.is_enum = true;
            goto end_range;
        }
        *
        {
            return false;
        }
        */
    }

end_range: ;

    //--------------------------------------------------------------------------
    // Parse Description

    slider.desc = ysfx::trim(YYCURSOR, &ysfx::ascii_isspace);
    if (slider.desc.empty())
        return false;

    return true;
}
