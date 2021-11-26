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

#include "ysfx.hpp"
#include "ysfx_parse.hpp"
#include "ysfx_test_utils.hpp"
#include <catch.hpp>

TEST_CASE("section splitting", "[parse]")
{
    SECTION("sections 1")
    {
        const char *text =
            "// the header" "\n"
            "@init" "\n"
            "the init" "\n"
            "@slider" "\n"
            "the slider, part 1" "\n"
            "the slider, part 2" "\n"
            "@block" "\n"
            "the block" "\n";
        ysfx::string_text_reader reader(text);

        ysfx_parse_error err;
        ysfx_toplevel_t toplevel;
        REQUIRE(ysfx_parse_toplevel(reader, toplevel, &err));
        REQUIRE(!err);

        REQUIRE(toplevel.header);
        REQUIRE(toplevel.init);
        REQUIRE(toplevel.slider);
        REQUIRE(toplevel.block);
        REQUIRE(!toplevel.sample);
        REQUIRE(!toplevel.serialize);
        REQUIRE(!toplevel.gfx);

        REQUIRE(toplevel.header->line_offset == 0);
        REQUIRE(toplevel.header->text == "// the header" "\n");
        REQUIRE(toplevel.init->line_offset == 2);
        REQUIRE(toplevel.init->text == "the init" "\n");
        REQUIRE(toplevel.slider->line_offset == 4);
        REQUIRE(toplevel.slider->text == "the slider, part 1" "\n" "the slider, part 2" "\n");
        REQUIRE(toplevel.block->line_offset == 7);
        REQUIRE(toplevel.block->text == "the block" "\n");
    }

    SECTION("sections 2")
    {
        const char *text =
            "// the header" "\n"
            "@sample" "\n"
            "the sample" "\n"
            "@serialize" "\n"
            "the serialize" "\n"
            "@gfx" "\n"
            "the gfx" "\n";
        ysfx::string_text_reader reader(text);

        ysfx_parse_error err;
        ysfx_toplevel_t toplevel;
        REQUIRE(ysfx_parse_toplevel(reader, toplevel, &err));
        REQUIRE(!err);

        REQUIRE(toplevel.header);
        REQUIRE(!toplevel.init);
        REQUIRE(!toplevel.slider);
        REQUIRE(!toplevel.block);
        REQUIRE(toplevel.sample);
        REQUIRE(toplevel.serialize);
        REQUIRE(toplevel.gfx);

        REQUIRE(toplevel.header->line_offset == 0);
        REQUIRE(toplevel.header->text == "// the header" "\n");
        REQUIRE(toplevel.sample->line_offset == 2);
        REQUIRE(toplevel.sample->text == "the sample" "\n");
        REQUIRE(toplevel.serialize->line_offset == 4);
        REQUIRE(toplevel.serialize->text == "the serialize" "\n");
        REQUIRE(toplevel.gfx->line_offset == 6);
        REQUIRE(toplevel.gfx->text == "the gfx" "\n");
    }

    SECTION("empty")
    {
        const char *text = "";
        ysfx::string_text_reader reader(text);

        ysfx_parse_error err;
        ysfx_toplevel_t toplevel;
        REQUIRE(ysfx_parse_toplevel(reader, toplevel, &err));
        REQUIRE(!err);

        // toplevel always has a header, empty or not

        REQUIRE(toplevel.header);
        REQUIRE(!toplevel.init);
        REQUIRE(!toplevel.slider);
        REQUIRE(!toplevel.block);
        REQUIRE(!toplevel.sample);
        REQUIRE(!toplevel.serialize);
        REQUIRE(!toplevel.gfx);

        REQUIRE(toplevel.header->line_offset == 0);
        REQUIRE(toplevel.header->text.empty());
    }

    SECTION("unrecognized section")
    {
        const char *text = "@abc";
        ysfx::string_text_reader reader(text);

        ysfx_parse_error err;
        ysfx_toplevel_t toplevel;
        REQUIRE(!ysfx_parse_toplevel(reader, toplevel, &err));
        REQUIRE(bool(err));
    }

    SECTION("trailing garbage")
    {
        const char *text = "@init zzz";
        ysfx::string_text_reader reader(text);

        ysfx_parse_error err;
        ysfx_toplevel_t toplevel;
        REQUIRE(ysfx_parse_toplevel(reader, toplevel, &err));
        REQUIRE(!err);

        REQUIRE(toplevel.init);
    }

    SECTION("gfx dimensions (default)")
    {
        const char *text = "@gfx";
        ysfx::string_text_reader reader(text);

        ysfx_parse_error err;
        ysfx_toplevel_t toplevel;
        REQUIRE(ysfx_parse_toplevel(reader, toplevel, &err));
        REQUIRE(!err);

        REQUIRE(toplevel.gfx);
        REQUIRE(toplevel.gfx_w == 0);
        REQUIRE(toplevel.gfx_h == 0);
    }

    SECTION("gfx dimensions (both)")
    {
        const char *text = "@gfx 123 456";
        ysfx::string_text_reader reader(text);

        ysfx_parse_error err;
        ysfx_toplevel_t toplevel;
        REQUIRE(ysfx_parse_toplevel(reader, toplevel, &err));
        REQUIRE(!err);

        REQUIRE(toplevel.gfx);
        REQUIRE(toplevel.gfx_w == 123);
        REQUIRE(toplevel.gfx_h == 456);
    }

    SECTION("gfx dimensions (just one)")
    {
        const char *text = "@gfx 123";
        ysfx::string_text_reader reader(text);

        ysfx_parse_error err;
        ysfx_toplevel_t toplevel;
        REQUIRE(ysfx_parse_toplevel(reader, toplevel, &err));
        REQUIRE(!err);

        REQUIRE(toplevel.gfx);
        REQUIRE(toplevel.gfx_w == 123);
        REQUIRE(toplevel.gfx_h == 0);
    }

    SECTION("gfx dimensions (garbage)")
    {
        const char *text = "@gfx aa bb cc";
        ysfx::string_text_reader reader(text);

        ysfx_parse_error err;
        ysfx_toplevel_t toplevel;
        REQUIRE(ysfx_parse_toplevel(reader, toplevel, &err));
        REQUIRE(!err);

        REQUIRE(toplevel.gfx);
        REQUIRE(toplevel.gfx_w == 0);
        REQUIRE(toplevel.gfx_h == 0);
    }
}

//------------------------------------------------------------------------------
static void ensure_basic_slider(const ysfx_slider_t &slider, uint32_t id, const std::string &var, const std::string &desc)
{
    REQUIRE(slider.id == id);
    if (!var.empty())
        REQUIRE(slider.var == var);
    else
        REQUIRE(slider.var == "slider" + std::to_string(id + 1));
    if (!desc.empty())
        REQUIRE(slider.desc == desc);
}

static void ensure_regular_slider(const ysfx_slider_t &slider, uint32_t id, const std::string &var, const std::string &desc, ysfx_real def, ysfx_real min, ysfx_real max, ysfx_real inc)
{
    ensure_basic_slider(slider, id, var, desc);
    REQUIRE(slider.def == Approx(def));
    REQUIRE(slider.min == Approx(min));
    REQUIRE(slider.max == Approx(max));
    REQUIRE(slider.inc == Approx(inc));
    REQUIRE(!slider.is_enum);
    REQUIRE(slider.enum_names.empty());
    REQUIRE(slider.path.empty());
}

static void ensure_enum_slider(const ysfx_slider_t &slider, uint32_t id, const std::string &var, const std::string &desc, ysfx_real def, const std::vector<std::string> &enums)
{
    ensure_basic_slider(slider, id, var, desc);
    REQUIRE(slider.def == Approx(def));
    REQUIRE(slider.min == 0);
    REQUIRE(slider.max == enums.size() - 1);
    REQUIRE(slider.inc == 1);
    REQUIRE(slider.is_enum);
    REQUIRE(slider.enum_names == enums);
    REQUIRE(slider.path.empty());
}

static void ensure_path_slider(const ysfx_slider_t &slider, uint32_t id, const std::string &var, const std::string &desc, ysfx_real def, const std::string &path)
{
    ensure_basic_slider(slider, id, var, desc);
    REQUIRE(slider.def == Approx(def));
    REQUIRE(slider.min == 0);
    REQUIRE(slider.max == 0);
    REQUIRE(slider.inc == 1);
    REQUIRE(slider.is_enum);
    REQUIRE(slider.enum_names.empty());
    if (!path.empty())
        REQUIRE(slider.path == path);
    else
        REQUIRE(!slider.path.empty());
}

TEST_CASE("slider parsing", "[parse]")
{
    SECTION("minimal range syntax")
    {
        const char *line = "slider43:123,Cui cui";
        ysfx_slider_t slider;
        REQUIRE(ysfx_parse_slider(line, slider));
        ensure_regular_slider(slider, 42, {}, "Cui cui", 123, 0, 0, 0);
    }

    SECTION("slider 0 invalid")
    {
        const char *line = "slider0:123,Cui cui";
        ysfx_slider_t slider;
        REQUIRE(!ysfx_parse_slider(line, slider));
    }

    SECTION("normal range syntax (no min-max-inc, no enum)")
    {
        const char *line = "slider43:123.1,Cui cui";
        ysfx_slider_t slider;
        REQUIRE(ysfx_parse_slider(line, slider));
        ensure_regular_slider(slider, 42, {}, "Cui cui", 123.1, 0, 0, 0);
    }

    SECTION("normal range syntax (no min-max-inc (2), no enum)")
    {
        const char *line = "slider43:123.1<>,Cui cui";
        ysfx_slider_t slider;
        REQUIRE(ysfx_parse_slider(line, slider));
        ensure_regular_slider(slider, 42, {}, "Cui cui", 123.1, 0, 0, 0);
    }

    SECTION("normal range syntax (min-max-inc, no enum)")
    {
        const char *line = "slider43:123.1<45.2,67.3,89.4>Cui cui";
        ysfx_slider_t slider;
        REQUIRE(ysfx_parse_slider(line, slider));
        ensure_regular_slider(slider, 42, {}, "Cui cui", 123.1, 45.2, 67.3, 89.4);
    }

    SECTION("path syntax")
    {
        const char *line = "slider43:/titi:777:Cui cui";
        ysfx_slider_t slider;
        REQUIRE(ysfx_parse_slider(line, slider));
        ensure_path_slider(slider, 42, {}, "Cui cui", 777, "/titi");
    }

    SECTION("enum syntax")
    {
        const char *line = "slider5:0<0,2,1{LP,BP,HP}>Type";
        ysfx_slider_t slider;
        REQUIRE(ysfx_parse_slider(line, slider));
        ensure_enum_slider(slider, 4, {}, "Type", 0, {"LP", "BP", "HP"});
    }

    SECTION("enum syntax, permissive")
    {
        const char *line = "slider5:0<0,2,1<{LP,BP,HP}>Type";
        ysfx_slider_t slider;
        REQUIRE(ysfx_parse_slider(line, slider));
        ensure_enum_slider(slider, 4, {}, "Type", 0, {"LP", "BP", "HP"});
    }
}

TEST_CASE("header parsing", "[parse]")
{
    SECTION("ordinary header", "[parse]")
    {
        const char *text =
            "desc:The desc" "\n"
            "in_pin:The input 1" "\n"
            "in_pin:The input 2" "\n"
            "out_pin:The output 1" "\n"
            "out_pin:The output 2" "\n"
            "slider43:123.1<45.2,67.3,89.4>Cui cui" "\n"
            "import foo.jsfx-inc" "\n";

        ysfx_section_t section;
        section.line_offset = 0;
        section.text.assign(text);

        ysfx_header_t header;
        ysfx_parse_header(&section, header);
        REQUIRE(header.desc == "The desc");
        REQUIRE(header.in_pins.size() == 2);
        REQUIRE(header.in_pins[0] == "The input 1");
        REQUIRE(header.in_pins[1] == "The input 2");
        REQUIRE(header.out_pins.size() == 2);
        REQUIRE(header.out_pins[0] == "The output 1");
        REQUIRE(header.out_pins[1] == "The output 2");
        REQUIRE(header.sliders[42].exists);
        REQUIRE(header.imports.size() == 1);
        REQUIRE(header.imports[0] == "foo.jsfx-inc");
    }

    SECTION("explicit pins to none", "[parse]")
    {
        const char *text =
            "in_pin:none" "\n"
            "out_pin:none" "\n";

        ysfx_section_t section;
        section.line_offset = 0;
        section.text.assign(text);

        ysfx_header_t header;
        ysfx_parse_header(&section, header);
        REQUIRE(header.in_pins.empty());
        REQUIRE(header.out_pins.empty());
    }

    SECTION("explicit pins to none, case sensitive", "[parse]")
    {
        const char *text =
            "in_pin:nOnE" "\n"
            "out_pin:NoNe" "\n";

        ysfx_section_t section;
        section.line_offset = 0;
        section.text.assign(text);

        ysfx_header_t header;
        ysfx_parse_header(&section, header);
        REQUIRE(header.in_pins.empty());
        REQUIRE(header.out_pins.empty());
    }

    SECTION("multiple pins with none", "[parse]")
    {
        const char *text =
            "in_pin:none" "\n"
            "in_pin:Input" "\n"
            "out_pin:Output" "\n"
            "out_pin:none" "\n";

        ysfx_section_t section;
        section.line_offset = 0;
        section.text.assign(text);

        ysfx_header_t header;
        ysfx_parse_header(&section, header);
        REQUIRE(header.in_pins.size() == 2);
        REQUIRE(header.in_pins[0] == "none");
        REQUIRE(header.in_pins[1] == "Input");
        REQUIRE(header.out_pins.size() == 2);
        REQUIRE(header.out_pins[0] == "Output");
        REQUIRE(header.out_pins[1] == "none");
    }

    SECTION("unspecified pins with @sample", "[parse]")
    {
        const char *text =
            "desc:Example" "\n"
            "@sample" "\n"
            "donothing();" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));

        ysfx_header_t &header = fx->source.main->header;
        REQUIRE(header.in_pins.size() == 2);
        REQUIRE(header.out_pins.size() == 2);
    }

    SECTION("unspecified pins without @sample", "[parse]")
    {
        const char *text =
            "desc:Example" "\n"
            "@block" "\n"
            "donothing();" "\n";

        scoped_new_dir dir_fx("${root}/Effects");
        scoped_new_txt file_main("${root}/Effects/example.jsfx", text);

        ysfx_config_u config{ysfx_config_new()};
        ysfx_u fx{ysfx_new(config.get())};

        REQUIRE(ysfx_load_file(fx.get(), file_main.m_path.c_str(), 0));

        ysfx_header_t &header = fx->source.main->header;
        REQUIRE(header.in_pins.size() == 0);
        REQUIRE(header.out_pins.size() == 0);
    }

    SECTION("filenames", "[parse]")
    {
        const char *text =
            "filename:0,toto" "\n"
            "filename:1,titi" "\n"
            "filename:2,tata" "\n";

        ysfx_section_t section;
        section.line_offset = 0;
        section.text.assign(text);

        ysfx_header_t header;
        ysfx_parse_header(&section, header);
        REQUIRE(header.filenames.size() == 3);
        REQUIRE(header.filenames[0] == "toto");
        REQUIRE(header.filenames[1] == "titi");
        REQUIRE(header.filenames[2] == "tata");
    }

    SECTION("out-of-order filenames", "[parse]")
    {
        const char *text =
            "filename:0,toto" "\n"
            "filename:2,tata" "\n"
            "filename:1,titi" "\n";

        ysfx_section_t section;
        section.line_offset = 0;
        section.text.assign(text);

        ysfx_header_t header;
        ysfx_parse_header(&section, header);
        REQUIRE(header.filenames.size() == 2);
        REQUIRE(header.filenames[0] == "toto");
        REQUIRE(header.filenames[1] == "titi");
    }
}
