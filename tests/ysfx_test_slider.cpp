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

#include "ysfx.h"
#include "ysfx_test_utils.hpp"
#include <catch.hpp>

TEST_CASE("slider manipulation", "[sliders]")
{
    SECTION("slider aliases")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "slider1:foo=1<1,3,0.1>the slider 1" "\n"
            "slider2:bar=2<1,3,0.1>the slider 2" "\n"
            "@init" "\n"
            "foo=2;" "\n"
            "bar=3;" "\n"
            "@sample" "\n"
            "spl0=0.0;" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        REQUIRE(ysfx_slider_get_value(fx.get(), 0) == 1);
        REQUIRE(ysfx_slider_get_value(fx.get(), 1) == 2);
        ysfx_init(fx.get());
        REQUIRE(ysfx_slider_get_value(fx.get(), 0) == 2);
        REQUIRE(ysfx_slider_get_value(fx.get(), 1) == 3);
    }
}
