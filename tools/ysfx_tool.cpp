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
#include <getopt.h>
#include <vector>
#include <string>
#include <chrono>
#include <cstdio>
#include <cstdlib>
namespace kro = std::chrono;

struct {
    const char *input_file = nullptr;
    bool no_gfx = false;
    bool no_serialize = false;
} args;

void print_help()
{
    fprintf(stderr, "Usage: ysfx_tool [option]... <file.jsfx>\n"
        "Options:\n"
        "\t" "--no-gfx          Do not compile the @gfx section" "\n"
        "\t" "--no-serialize    Do not compile the @serialize section" "\n");
}

void process_args(int argc, char *argv[])
{
    const struct option longopts[] = {
        {"help", 0, nullptr, 'h'},
        {"no-gfx", 0, nullptr, 'G'},
        {"no-serialize", 0, nullptr, 'S'},
        {},
    };

    if (argc < 2) {
        print_help();
        exit(0);
    }

    for (int c; (c = getopt_long(argc, argv, "h", longopts, nullptr)) != -1;) {
        switch (c) {
        case 'h':
            print_help();
            exit(0);
        case 'G':
            args.no_gfx = true;
            break;
        case 'S':
            args.no_serialize = true;
            break;
        default:
            exit(1);
        }
    }

    if (argc - optind != 1) {
        fprintf(stderr, "Please specify exactly one input file.\n");
        exit(1);
    }

    args.input_file = argv[optind];
}

void log_report(intptr_t userdata, ysfx_log_level level, const char *message)
{
    printf(" %10s -> %s\n", ysfx_log_level_string(level), message);
}

const char *yesno(bool b)
{
    return b ? "yes" : "no";
}

void dump_header_info(ysfx_t *fx)
{
    printf("\n" "--- header information ---" "\n\n");

    printf("Name: %s\n", ysfx_get_name(fx));
    printf("Author: %s\n", ysfx_get_author(fx));

    uint32_t tag_count = ysfx_get_num_tags(fx);
    std::vector<const char *> tags(tag_count);
    ysfx_get_tags(fx, tags.data(), tag_count);
    printf("Tags: %u\n", tag_count);
    for (uint32_t i = 0; i < tag_count; ++i)
        printf("\t* Name: %s\n", tags[i]);

    std::vector<std::string> sections;
    sections.reserve(8);
    if (ysfx_has_section(fx, ysfx_section_init))
        sections.push_back("@init");
    if (ysfx_has_section(fx, ysfx_section_slider))
        sections.push_back("@slider");
    if (ysfx_has_section(fx, ysfx_section_block))
        sections.push_back("@block");
    if (ysfx_has_section(fx, ysfx_section_sample))
        sections.push_back("@sample");
    if (ysfx_has_section(fx, ysfx_section_gfx))
        sections.push_back("@gfx");
    if (ysfx_has_section(fx, ysfx_section_serialize))
        sections.push_back("@serialize");
    printf("Sections: %u\n", (uint32_t)sections.size());
    for (const std::string &section : sections)
        printf("\t* ID: %s\n", section.c_str());

    const uint32_t num_ins = ysfx_get_num_inputs(fx);
    const uint32_t num_outs = ysfx_get_num_outputs(fx);
    printf("Inputs: %u\n", num_ins);
    for (uint32_t i = 0; i < num_ins; ++i) {
        printf("\t* ID: %u\n", i);
        printf("\t  Name: %s\n", ysfx_get_input_name(fx, i));
    }
    printf("Outputs: %u\n", num_outs);
    for (uint32_t i = 0; i < num_outs; ++i) {
        printf("\t* ID: %u\n", i);
        printf("\t  Name: %s\n", ysfx_get_output_name(fx, i));
    }

    uint32_t num_sliders = 0;
    for (uint32_t i = 0; i < ysfx_max_sliders; ++i)
        num_sliders += ysfx_slider_exists(fx, i);
    printf("Sliders: %u\n", num_sliders);

    for (uint32_t i = 0; i < ysfx_max_sliders; ++i) {
        if (!ysfx_slider_exists(fx, i))
            continue;
        printf("\t* ID: %u\n", i);
        printf("\t  Name: %s\n", ysfx_slider_get_name(fx, i));
        ysfx_slider_range_t range{};
        ysfx_slider_get_range(fx, i, &range);
        printf("\t  Default: %f\n", range.def);
        printf("\t  Minimum: %f\n", range.min);
        printf("\t  Maximum: %f\n", range.max);
        printf("\t  Increment: %f\n", range.inc);
        bool is_enum = ysfx_slider_is_enum(fx, i);
        bool is_path = ysfx_slider_is_path(fx, i);
        printf("\t  Enumeration: %s\n", yesno(is_enum));
        printf("\t  Path: %s\n", yesno(is_path));
        if (is_enum) {
            uint32_t count = ysfx_slider_get_enum_size(fx, i);
            std::vector<const char *> names(count);
            ysfx_slider_get_enum_names(fx, i, names.data(), count);
            for (uint32_t j = 0; j < count; ++j)
                printf("\t\t* Name: %s\n", names[j]);
        }
    }
}

bool process_jsfx()
{
    ysfx_config_u config{ysfx_config_new()};
    ysfx_set_log_reporter(config.get(), &log_report);
    ysfx_register_builtin_audio_formats(config.get());

    printf("* File: %s" "\n", args.input_file);

    ysfx_guess_file_roots(config.get(), args.input_file);

    printf("* Import root: %s\n", ysfx_get_import_root(config.get()));
    printf("* Data root: %s\n", ysfx_get_data_root(config.get()));

    kro::steady_clock::time_point t1, t2;

    printf("\n" "--- loading ---" "\n\n");

    ysfx_u fx{ysfx_new(config.get())};
    t1 = kro::steady_clock::now();
    if (!ysfx_load_file(fx.get(), args.input_file, 0))
        return false;
    t2 = kro::steady_clock::now();
    printf("Elapsed: %.3f ms\n", 1e3 * kro::duration<double>(t2 - t1).count());

    dump_header_info(fx.get());

    printf("\n" "--- compilation ---" "\n\n");

    t1 = kro::steady_clock::now();
    uint32_t compile_opts = 0;
    if (args.no_gfx)
        compile_opts |= ysfx_compile_no_gfx;
    if (args.no_serialize)
        compile_opts |= ysfx_compile_no_serialize;
    if (!ysfx_compile(fx.get(), compile_opts))
        return false;
    t2 = kro::steady_clock::now();
    printf("Elapsed: %.3f ms\n", 1e3 * kro::duration<double>(t2 - t1).count());

    printf("\n" "--- success ---" "\n");
    return true;
}

int main(int argc, char *argv[])
{
    process_args(argc, argv);

    if (!process_jsfx())
        return 1;

    return 0;
}
