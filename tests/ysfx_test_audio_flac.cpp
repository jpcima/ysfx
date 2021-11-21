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
#include "ysfx_audio_flac.hpp"
#include "ysfx_utils.hpp"
#include <catch.hpp>
#include <random>

#if defined(YSFX_TESTS_HAVE_SNDFILE)
#   include <sndfile.h>
#endif

TEST_CASE("flac audio format", "[flac]")
{
#if !defined(YSFX_TESTS_HAVE_SNDFILE)
    WARN("sndfile is missing, not testing flac");
#else
    SECTION("read flac file")
    {
        scoped_new_txt flac_file("${root}/example.flac", nullptr, 0);
        REQUIRE(ysfx_audio_format_flac.can_handle(flac_file.m_path.c_str()));

        uint32_t channels = 8;
        ysfx_real sampleRate = 44100;
        uint64_t totalframes = 1024;
        uint64_t totalsmpls = channels * totalframes;
        std::unique_ptr<float[]> data{new float[(size_t)totalsmpls]};

        {
            std::mt19937_64 prng;
            for (size_t i = 0; i < (size_t)totalsmpls; ++i)
                data[i] = std::uniform_real_distribution<float>{-1.0f, 1.0f}(prng);
        }

        {
            SF_INFO info{};
            info.frames = totalframes;
            info.samplerate = sampleRate;
            info.channels = channels;
            info.format = SF_FORMAT_FLAC|SF_FORMAT_PCM_16;
            SNDFILE *snd = sf_open(flac_file.m_path.c_str(), SFM_WRITE, &info);
            REQUIRE(snd);
            REQUIRE(sf_writef_float(snd, data.get(), totalframes) == (sf_count_t)totalframes);
            sf_close(snd);
        }

        // try reading in various buffer sizes
        for (uint32_t bufsize = 1; bufsize <= channels; ++bufsize) {
            ysfx_audio_reader_t *reader = ysfx_audio_format_flac.open(flac_file.m_path.c_str());
            REQUIRE(reader);
            auto reader_cleanup = ysfx::defer([reader]() { ysfx_audio_format_flac.close(reader); });

            std::unique_ptr<ysfx_real[]> buf{new ysfx_real[bufsize]};

            ysfx_audio_file_info_t info = ysfx_audio_format_flac.info(reader);
            REQUIRE(info.sample_rate == sampleRate);
            REQUIRE(info.channels == channels);

            // do once, and redo after rewind
            for (int time = 0; time < 2; ++time) {
                uint64_t smplpos = 0;

                while (smplpos < totalsmpls) {
                    REQUIRE(ysfx_audio_format_flac.avail(reader) == totalsmpls - smplpos);
                    uint64_t n = totalsmpls - smplpos;
                    if (n > bufsize)
                        n = bufsize;
                    REQUIRE(ysfx_audio_format_flac.read(reader, buf.get(), n) == n);
                    for (size_t i = 0; i < n; ++i)
                        REQUIRE(data[(size_t)smplpos + i] == Approx(buf[i]).margin(1e-3));
                    smplpos += n;
                }

                ysfx_audio_format_flac.rewind(reader);
            }
        }
    }
#endif
}
