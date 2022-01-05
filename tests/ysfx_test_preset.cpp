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
#include <cstring>

TEST_CASE("preset handling", "[preset]")
{
    SECTION("Bank from RPL")
    {
        const char *source_text =
            "desc:TestCaseRPL" "\n"
            "slider1:0<0,1,0.01>S1" "\n"
            "slider2:0<0,1,0.01>S2" "\n"
            "slider4:0<0,1,0.01>S4" "\n"
            "@serialize" "\n"
            "file_var(0, slider4);" "\n"
            "file_var(0, slider2);" "\n"
            "file_var(0, slider1);" "\n";

        const char *rpl_text =
            "<REAPER_PRESET_LIBRARY \"JS: TestCaseRPL\"" "\n"
            "  <PRESET `1.defaults`" "\n"
            "    MCAwIC0gMCAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0g" "\n"
            "    LSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAxLmRlZmF1bHRzAAAAAAAAAAAAAAAAAA==" "\n"
            "  >" "\n"
            "  <PRESET `2.a preset with spaces in the name`" "\n"
            "    MC4zNCAwLjc1IC0gMC42MiAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAt" "\n"
            "    IC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAiMi5hIHByZXNldCB3aXRoIHNwYWNlcyBpbiB0aGUgbmFtZSIAUrgePwAAQD97FK4+" "\n"
            "  >" "\n"
            "  <PRESET `3.a preset with \"quotes\" in the name`" "\n"
            "    MC44NiAwLjA3IC0gMC4yNSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAt" "\n"
            "    IC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAnMy5hIHByZXNldCB3aXRoICJxdW90ZXMiIGluIHRoZSBuYW1lJwAAAIA+KVyPPfYoXD8=" "\n"
            "  >" "\n"
            "  <PRESET `>`" "\n"
            "    MSAwLjkgLSAwLjggLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0g" "\n"
            "    LSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gLSAtIC0gPgDNzEw/ZmZmPwAAgD8=" "\n"
            "  >" "\n"
            ">" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", source_text);
        scoped_new_txt file_rpl("${root}/Effects/example.jsfx.rpl", rpl_text);

        ysfx_bank_u bank{ysfx_load_bank_from_rpl(file_rpl.m_path.c_str())};
        REQUIRE(bank);

        REQUIRE(!strcmp(bank->name, "JS: TestCaseRPL"));

        REQUIRE(bank->presets != nullptr);
        REQUIRE(bank->preset_count == 4);

        ysfx_preset_t *preset;
        ysfx_state_t *state;

        preset = &bank->presets[0];
        REQUIRE(!strcmp(preset->name, "1.defaults"));
        state = preset->state;
        REQUIRE(state->slider_count == 3);
        REQUIRE(state->sliders[0].index == 0);
        REQUIRE(state->sliders[0].value == 0.0);
        REQUIRE(state->sliders[1].index == 1);
        REQUIRE(state->sliders[1].value == 0.0);
        REQUIRE(state->sliders[2].index == 3);
        REQUIRE(state->sliders[2].value == 0.0);
        REQUIRE(state->data_size == 3 * sizeof(float));
        REQUIRE(ysfx::unpack_f32le(&state->data[0 * sizeof(float)]) == 0.0);
        REQUIRE(ysfx::unpack_f32le(&state->data[1 * sizeof(float)]) == 0.0);
        REQUIRE(ysfx::unpack_f32le(&state->data[2 * sizeof(float)]) == 0.0);

        preset = &bank->presets[1];
        REQUIRE(!strcmp(preset->name, "2.a preset with spaces in the name"));
        state = preset->state;
        REQUIRE(state->slider_count == 3);
        REQUIRE(state->sliders[0].index == 0);
        REQUIRE(state->sliders[0].value == Approx(0.34));
        REQUIRE(state->sliders[1].index == 1);
        REQUIRE(state->sliders[1].value == Approx(0.75));
        REQUIRE(state->sliders[2].index == 3);
        REQUIRE(state->sliders[2].value == Approx(0.62));
        REQUIRE(state->data_size == 3 * sizeof(float));
        REQUIRE(ysfx::unpack_f32le(&state->data[0 * sizeof(float)]) == Approx(0.62));
        REQUIRE(ysfx::unpack_f32le(&state->data[1 * sizeof(float)]) == Approx(0.75));
        REQUIRE(ysfx::unpack_f32le(&state->data[2 * sizeof(float)]) == Approx(0.34));

        preset = &bank->presets[2];
        REQUIRE(!strcmp(preset->name, "3.a preset with \"quotes\" in the name"));
        state = preset->state;
        REQUIRE(state->slider_count == 3);
        REQUIRE(state->sliders[0].index == 0);
        REQUIRE(state->sliders[0].value == Approx(0.86));
        REQUIRE(state->sliders[1].index == 1);
        REQUIRE(state->sliders[1].value == Approx(0.07));
        REQUIRE(state->sliders[2].index == 3);
        REQUIRE(state->sliders[2].value == Approx(0.25));
        REQUIRE(state->data_size == 3 * sizeof(float));
        REQUIRE(ysfx::unpack_f32le(&state->data[0 * sizeof(float)]) == Approx(0.25));
        REQUIRE(ysfx::unpack_f32le(&state->data[1 * sizeof(float)]) == Approx(0.07));
        REQUIRE(ysfx::unpack_f32le(&state->data[2 * sizeof(float)]) == Approx(0.86));

        preset = &bank->presets[3];
        REQUIRE(!strcmp(preset->name, ">"));
        state = preset->state;
        REQUIRE(state->slider_count == 3);
        REQUIRE(state->sliders[0].index == 0);
        REQUIRE(state->sliders[0].value == Approx(1.0));
        REQUIRE(state->sliders[1].index == 1);
        REQUIRE(state->sliders[1].value == Approx(0.9));
        REQUIRE(state->sliders[2].index == 3);
        REQUIRE(state->sliders[2].value == Approx(0.8));
        REQUIRE(state->data_size == 3 * sizeof(float));
        REQUIRE(ysfx::unpack_f32le(&state->data[0 * sizeof(float)]) == Approx(0.8));
        REQUIRE(ysfx::unpack_f32le(&state->data[1 * sizeof(float)]) == Approx(0.9));
        REQUIRE(ysfx::unpack_f32le(&state->data[2 * sizeof(float)]) == Approx(1.0));
    }

    SECTION("Locate preset bank")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "@sample" "\n"
            "spl0=0.0;" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        {
            REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
            REQUIRE(ysfx_get_bank_path(fx.get()) == std::string());
        }

        {
            scoped_new_txt file_rpl("${root}/Effects/example.jsfx.rpl", "");
            REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
            REQUIRE(ysfx_get_bank_path(fx.get()) == file_rpl.m_path);
        }

        {
            scoped_new_txt file_rpl("${root}/Effects/example.jsfx.RpL", "");
            REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
            REQUIRE(ysfx_get_bank_path(fx.get()) == file_rpl.m_path);
        }
    }
}
