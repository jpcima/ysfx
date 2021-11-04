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
#include "ysfx_utils.hpp"
#include "ysfx_test_utils.hpp"
#include <catch.hpp>

TEST_CASE("save and load", "[serialization]")
{
    SECTION("sliders only")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "slider1:1<1,3,0.1>the slider 1" "\n"
            "slider2:2<1,3,0.1>the slider 2" "\n"
            "@sample" "\n"
            "spl0=0.0;" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        ysfx_state_u state{ysfx_save_state(fx.get())};
        REQUIRE(state);
        REQUIRE(state->data_size == 0);
        REQUIRE(state->slider_count == 2);
        REQUIRE(state->sliders[0].index == 0);
        REQUIRE(state->sliders[1].index == 1);
        REQUIRE(state->sliders[0].value == 1);
        REQUIRE(state->sliders[1].value == 2);

        state->sliders[0].value = 2;
        state->sliders[1].value = 3;
        ysfx_load_state(fx.get(), state.get());
        REQUIRE(ysfx_slider_get_value(fx.get(), 0) == 2);
        REQUIRE(ysfx_slider_get_value(fx.get(), 1) == 3);
    };

    SECTION("serialization only")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "@init" "\n"
            "myvar1=1;" "\n"
            "myvar2=2;" "\n"
            "myarray=777;" "\n"
            "myarray[0]=100;" "\n"
            "myarray[1]=200;" "\n"
            "myarray[2]=300;" "\n"
            "@serialize" "\n"
            "file_var(0, myvar1);" "\n"
            "file_var(0, myvar2);" "\n"
            "file_mem(0, myarray, 3);" "\n"
            "@sample" "\n"
            "spl0=0.0;" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        ysfx_state_u state{ysfx_save_state(fx.get())};
        REQUIRE(state);
        REQUIRE(state->data_size == 5 * sizeof(float));
        REQUIRE(ysfx::unpack_f32le(&state->data[0 * sizeof(float)]) == 1);
        REQUIRE(ysfx::unpack_f32le(&state->data[1 * sizeof(float)]) == 2);
        REQUIRE(ysfx::unpack_f32le(&state->data[2 * sizeof(float)]) == 100);
        REQUIRE(ysfx::unpack_f32le(&state->data[3 * sizeof(float)]) == 200);
        REQUIRE(ysfx::unpack_f32le(&state->data[4 * sizeof(float)]) == 300);

        ysfx::pack_f32le(2, &state->data[0 * sizeof(float)]);
        ysfx::pack_f32le(3, &state->data[1 * sizeof(float)]);
        ysfx::pack_f32le(200, &state->data[2 * sizeof(float)]);
        ysfx::pack_f32le(300, &state->data[3 * sizeof(float)]);
        ysfx::pack_f32le(400, &state->data[4 * sizeof(float)]);
        ysfx_load_state(fx.get(), state.get());
        state.reset(ysfx_save_state(fx.get()));
        REQUIRE(state->data_size == 5 * sizeof(float));
        REQUIRE(ysfx::unpack_f32le(&state->data[0 * sizeof(float)]) == 2);
        REQUIRE(ysfx::unpack_f32le(&state->data[1 * sizeof(float)]) == 3);
        REQUIRE(ysfx::unpack_f32le(&state->data[2 * sizeof(float)]) == 200);
        REQUIRE(ysfx::unpack_f32le(&state->data[3 * sizeof(float)]) == 300);
        REQUIRE(ysfx::unpack_f32le(&state->data[4 * sizeof(float)]) == 400);
    };
}
