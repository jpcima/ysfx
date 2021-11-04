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
#include "ysfx_audio_wav.hpp"
#include "ysfx_utils.hpp"
#include <catch.hpp>
#include <random>

#if defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define DR_WAV_IMPLEMENTATION
#define DRWAV_API static
#define DRWAV_PRIVATE static
#include "dr_wav.h"

#if defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif

TEST_CASE("wav audio format", "[wav]")
{
    SECTION("read wav file")
    {
        scoped_new_txt wav_file("${root}/example.wav", nullptr, 0);
        REQUIRE(ysfx_audio_format_wav.can_handle(wav_file.m_path.c_str()));

        drwav_data_format fmt{};
        fmt.container = drwav_container_riff;
        fmt.format = DR_WAVE_FORMAT_IEEE_FLOAT;
        fmt.channels = 8;
        fmt.sampleRate = 44100;
        fmt.bitsPerSample = 32;
        uint64_t totalframes = 1024;
        uint64_t totalsmpls = fmt.channels * totalframes;
        std::unique_ptr<float[]> data{new float[(size_t)totalsmpls]};

        {
            std::mt19937_64 prng;
            for (size_t i = 0; i < (size_t)totalsmpls; ++i)
                data[i] = std::uniform_real_distribution<float>{-1.0f, 1.0f}(prng);
        }
        {
            drwav wav;
            REQUIRE(drwav_init_file_write(&wav, wav_file.m_path.c_str(), &fmt, nullptr));
            uint64_t written = drwav_write_pcm_frames(&wav, totalframes, data.get());
            drwav_uninit(&wav);
            REQUIRE(written == totalframes);
        }

        // try reading in various buffer sizes
        for (uint32_t bufsize = 1; bufsize <= fmt.channels; ++bufsize) {
            ysfx_audio_reader_t *reader = ysfx_audio_format_wav.open(wav_file.m_path.c_str());
            REQUIRE(reader);
            auto reader_cleanup = ysfx::defer([reader]() { ysfx_audio_format_wav.close(reader); });

            std::unique_ptr<ysfx_real[]> buf{new ysfx_real[bufsize]};

            ysfx_audio_file_info_t info = ysfx_audio_format_wav.info(reader);
            REQUIRE(info.sample_rate == fmt.sampleRate);
            REQUIRE(info.channels == fmt.channels);

            // do once, and redo after rewind
            for (int time = 0; time < 2; ++time) {
                uint64_t smplpos = 0;

                while (smplpos < totalsmpls) {
                    REQUIRE(ysfx_audio_format_wav.avail(reader) == totalsmpls - smplpos);
                    uint64_t n = totalsmpls - smplpos;
                    if (n > bufsize)
                        n = bufsize;
                    REQUIRE(ysfx_audio_format_wav.read(reader, buf.get(), n) == n);
                    for (size_t i = 0; i < n; ++i)
                        REQUIRE(data[(size_t)smplpos + i] == Approx(buf[i]));
                    smplpos += n;
                }

                ysfx_audio_format_wav.rewind(reader);
            }
        }
    }
}
