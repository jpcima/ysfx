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
#include <cstring>

TEST_CASE("midi input and output", "[midi]")
{
    SECTION("midisend")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "@block" "\n"
            "midisend(10, 0x90, 60, 0x40);" "\n"
            "midisend(20, 0x91, 61 + 256 * 0x7f);" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        ysfx_process_float(fx.get(), nullptr, nullptr, 0, 0, 30);

        ysfx_midi_event_t event;

        REQUIRE(ysfx_receive_midi(fx.get(), &event));
        REQUIRE(event.bus == 0);
        REQUIRE(event.offset == 10);
        REQUIRE(event.size == 3);
        REQUIRE(event.data[0] == 0x90);
        REQUIRE(event.data[1] == 60);
        REQUIRE(event.data[2] == 0x40);

        REQUIRE(ysfx_receive_midi(fx.get(), &event));
        REQUIRE(event.bus == 0);
        REQUIRE(event.offset == 20);
        REQUIRE(event.size == 3);
        REQUIRE(event.data[0] == 0x91);
        REQUIRE(event.data[1] == 61);
        REQUIRE(event.data[2] == 0x7f);

        REQUIRE(!ysfx_receive_midi(fx.get(), &event));
    }

    SECTION("midisend_buf")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "@init" "\n"
            "msg1 = 1000;" "\n"
            "msg1[0] = 0x90;" "\n"
            "msg1[1] = 60;" "\n"
            "msg1[2] = 0x40;" "\n"
            "msg2 = 2000;" "\n"
            "msg2[0] = 0x91;" "\n"
            "msg2[1] = 61;" "\n"
            "msg2[2] = 0x7f;" "\n"
            "@block" "\n"
            "midisend_buf(10, msg1, 3);" "\n"
            "midisend_buf(20, msg2, 3);" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        ysfx_process_float(fx.get(), nullptr, nullptr, 0, 0, 30);

        ysfx_midi_event_t event;

        REQUIRE(ysfx_receive_midi(fx.get(), &event));
        REQUIRE(event.bus == 0);
        REQUIRE(event.offset == 10);
        REQUIRE(event.size == 3);
        REQUIRE(event.data[0] == 0x90);
        REQUIRE(event.data[1] == 60);
        REQUIRE(event.data[2] == 0x40);

        REQUIRE(ysfx_receive_midi(fx.get(), &event));
        REQUIRE(event.bus == 0);
        REQUIRE(event.offset == 20);
        REQUIRE(event.size == 3);
        REQUIRE(event.data[0] == 0x91);
        REQUIRE(event.data[1] == 61);
        REQUIRE(event.data[2] == 0x7f);

        REQUIRE(!ysfx_receive_midi(fx.get(), &event));
    }

    SECTION("midisyx")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "@init" "\n"
            "msg1 = 1000;" "\n"
            "msg1[0] = 1;" "\n"
            "msg1[1] = 2;" "\n"
            "msg1[2] = 3;" "\n"
            "msg2 = 2000;" "\n"
            "msg2[0] = 0xf0;" "\n"
            "msg2[1] = 4;" "\n"
            "msg2[2] = 5;" "\n"
            "msg2[3] = 6;" "\n"
            "msg2[4] = 0xf7;" "\n"
            "@block" "\n"
            "midisyx(10, msg1, 3);" "\n"
            "midisyx(20, msg2, 5);" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        ysfx_process_float(fx.get(), nullptr, nullptr, 0, 0, 30);

        ysfx_midi_event_t event;

        REQUIRE(ysfx_receive_midi(fx.get(), &event));
        REQUIRE(event.bus == 0);
        REQUIRE(event.offset == 10);
        REQUIRE(event.size == 5);
        REQUIRE(event.data[0] == 0xf0);
        REQUIRE(event.data[1] == 1);
        REQUIRE(event.data[2] == 2);
        REQUIRE(event.data[3] == 3);
        REQUIRE(event.data[4] == 0xf7);

        REQUIRE(ysfx_receive_midi(fx.get(), &event));
        REQUIRE(event.bus == 0);
        REQUIRE(event.offset == 20);
        REQUIRE(event.size == 5);
        REQUIRE(event.data[0] == 0xf0);
        REQUIRE(event.data[1] == 4);
        REQUIRE(event.data[2] == 5);
        REQUIRE(event.data[3] == 6);
        REQUIRE(event.data[4] == 0xf7);
    }

    SECTION("midisend_str")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "@init" "\n"
            "msg1 = \"\x90\x3c\x40\";" "\n"
            "msg2 = \"\x91\x3d\x7f\";" "\n"
            "@block" "\n"
            "midisend_str(10, msg1);" "\n"
            "midisend_str(20, msg2);" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        ysfx_process_float(fx.get(), nullptr, nullptr, 0, 0, 30);

        ysfx_midi_event_t event;

        REQUIRE(ysfx_receive_midi(fx.get(), &event));
        REQUIRE(event.bus == 0);
        REQUIRE(event.offset == 10);
        REQUIRE(event.size == 3);
        REQUIRE(event.data[0] == 0x90);
        REQUIRE(event.data[1] == 60);
        REQUIRE(event.data[2] == 0x40);

        REQUIRE(ysfx_receive_midi(fx.get(), &event));
        REQUIRE(event.bus == 0);
        REQUIRE(event.offset == 20);
        REQUIRE(event.size == 3);
        REQUIRE(event.data[0] == 0x91);
        REQUIRE(event.data[1] == 61);
        REQUIRE(event.data[2] == 0x7f);

        REQUIRE(!ysfx_receive_midi(fx.get(), &event));
    }

    SECTION("midirecv")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "@init" "\n"
            "@block" "\n"
            "midirecv(aOff, a1, a2, a3);" "\n"
            "midirecv(bOff, b1, b2, b3);" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        ysfx_midi_event_t event;
        const uint8_t data1[] = {0x90, 60, 0x40};
        const uint8_t data2[] = {0x91, 61, 0x7f};

        event.bus = 0;
        event.offset = 10;
        event.size = 3;
        event.data = data1;
        REQUIRE(ysfx_send_midi(fx.get(), &event));

        event.bus = 0;
        event.offset = 20;
        event.size = 3;
        event.data = data2;
        REQUIRE(ysfx_send_midi(fx.get(), &event));

        ysfx_process_float(fx.get(), nullptr, nullptr, 0, 0, 30);

        ysfx_real *aOff, *a1, *a2, *a3;
        ysfx_real *bOff, *b1, *b2, *b3;
        REQUIRE((aOff = ysfx_find_var(fx.get(), "aOff")));
        REQUIRE((a1 = ysfx_find_var(fx.get(), "a1")));
        REQUIRE((a2 = ysfx_find_var(fx.get(), "a2")));
        REQUIRE((a3 = ysfx_find_var(fx.get(), "a3")));
        REQUIRE((bOff = ysfx_find_var(fx.get(), "bOff")));
        REQUIRE((b1 = ysfx_find_var(fx.get(), "b1")));
        REQUIRE((b2 = ysfx_find_var(fx.get(), "b2")));
        REQUIRE((b3 = ysfx_find_var(fx.get(), "b3")));

        REQUIRE(*aOff == 10);
        REQUIRE(*a1 == 0x90);
        REQUIRE(*a2 == 60);
        REQUIRE(*a3 == 0x40);
        REQUIRE(*bOff == 20);
        REQUIRE(*b1 == 0x91);
        REQUIRE(*b2 == 61);
        REQUIRE(*b3 == 0x7f);
    }

    SECTION("midirecv_buf")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "@init" "\n"
            "buf1 = 1000;" "\n"
            "buf2 = 2000;" "\n"
            "@block" "\n"
            "midirecv_buf(aOff, buf1, 3);" "\n"
            "midirecv_buf(bOff, buf2, 3);" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        ysfx_midi_event_t event;
        const uint8_t data1[] = {0x90, 60, 0x40};
        const uint8_t data2[] = {0x91, 61, 0x7f};

        event.bus = 0;
        event.offset = 10;
        event.size = 3;
        event.data = data1;
        REQUIRE(ysfx_send_midi(fx.get(), &event));

        event.bus = 0;
        event.offset = 20;
        event.size = 3;
        event.data = data2;
        REQUIRE(ysfx_send_midi(fx.get(), &event));

        ysfx_process_float(fx.get(), nullptr, nullptr, 0, 0, 30);

        ysfx_real *aOff, *bOff;
        REQUIRE((aOff = ysfx_find_var(fx.get(), "aOff")));
        REQUIRE((bOff = ysfx_find_var(fx.get(), "bOff")));

        ysfx_real *buf1, *buf2;
        REQUIRE((buf1 = ysfx_find_var(fx.get(), "buf1")));
        REQUIRE((buf2 = ysfx_find_var(fx.get(), "buf2")));

        ysfx_real mem1[3]{};
        ysfx_real mem2[3]{};
        ysfx_read_vmem(fx.get(), (uint32_t)*buf1, mem1, 3);
        ysfx_read_vmem(fx.get(), (uint32_t)*buf2, mem2, 3);

        REQUIRE(*aOff == 10);
        REQUIRE(mem1[0] == 0x90);
        REQUIRE(mem1[1] == 60);
        REQUIRE(mem1[2] == 0x40);
        REQUIRE(*bOff == 20);
        REQUIRE(mem2[0] == 0x91);
        REQUIRE(mem2[1] == 61);
        REQUIRE(mem2[2] == 0x7f);
    }

    SECTION("midirecv_str")
    {
        const char *text =
            "desc:example" "\n"
            "out_pin:output" "\n"
            "@init" "\n"
            "buf1 = 1000;" "\n"
            "buf2 = 2000;" "\n"
            "@block" "\n"
            "midirecv_str(aOff, #msg1);" "\n"
            "midirecv_str(bOff, #msg2);" "\n"
            "len1 = strlen(#msg1);" "\n"
            "len2 = strlen(#msg2);" "\n"
            "i = 0; while (i < len1) ( buf1[i] = str_getchar(#msg1, i, 'cu'); i = i + 1; );" "\n"
            "i = 0; while (i < len2) ( buf2[i] = str_getchar(#msg2, i, 'cu'); i = i + 1; );" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));
        REQUIRE(ysfx_compile(fx.get(), 0));

        ysfx_midi_event_t event;
        const uint8_t data1[] = {0x90, 60, 0x40};
        const uint8_t data2[] = {0x91, 61, 0x7f};

        event.bus = 0;
        event.offset = 10;
        event.size = 3;
        event.data = data1;
        REQUIRE(ysfx_send_midi(fx.get(), &event));

        event.bus = 0;
        event.offset = 20;
        event.size = 3;
        event.data = data2;
        REQUIRE(ysfx_send_midi(fx.get(), &event));

        ysfx_process_float(fx.get(), nullptr, nullptr, 0, 0, 30);

        ysfx_real *aOff, *bOff;
        REQUIRE((aOff = ysfx_find_var(fx.get(), "aOff")));
        REQUIRE((bOff = ysfx_find_var(fx.get(), "bOff")));

        ysfx_real *len1, *len2;
        REQUIRE((len1 = ysfx_find_var(fx.get(), "len1")));
        REQUIRE((len2 = ysfx_find_var(fx.get(), "len2")));

        REQUIRE(*len1 == 3);
        REQUIRE(*len2 == 3);

        ysfx_real *buf1, *buf2;
        REQUIRE((buf1 = ysfx_find_var(fx.get(), "buf1")));
        REQUIRE((buf2 = ysfx_find_var(fx.get(), "buf2")));

        ysfx_real mem1[3]{};
        ysfx_real mem2[3]{};
        ysfx_read_vmem(fx.get(), (uint32_t)*buf1, mem1, 3);
        ysfx_read_vmem(fx.get(), (uint32_t)*buf2, mem2, 3);

        REQUIRE(*aOff == 10);
        REQUIRE(mem1[0] == 0x90);
        REQUIRE(mem1[1] == 60);
        REQUIRE(mem1[2] == 0x40);
        REQUIRE(*bOff == 20);
        REQUIRE(mem2[0] == 0x91);
        REQUIRE(mem2[1] == 61);
        REQUIRE(mem2[2] == 0x7f);
    }

    // TODO test MIDI bus
}
