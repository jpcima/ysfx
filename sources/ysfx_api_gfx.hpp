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

#pragma once
#include "ysfx.h"
#include <thread>

#if !defined(YSFX_NO_GFX)
struct ysfx_gfx_state_t;
ysfx_gfx_state_t *ysfx_gfx_state_new();
void ysfx_gfx_state_free(ysfx_gfx_state_t *state);
YSFX_DEFINE_AUTO_PTR(ysfx_gfx_state_u, ysfx_gfx_state_t, ysfx_gfx_state_free);
void ysfx_gfx_state_set_thread(ysfx_gfx_state_t *state, std::thread::id id);
void ysfx_gfx_state_set_bitmap(ysfx_gfx_state_t *state, uint8_t *data, uint32_t w, uint32_t h, uint32_t stride);

//------------------------------------------------------------------------------
void ysfx_gfx_enter(ysfx_t *fx);
void ysfx_gfx_leave(ysfx_t *fx);
ysfx_gfx_state_t *ysfx_gfx_get_context(ysfx_t *fx);

struct ysfx_scoped_gfx_t {
    ysfx_scoped_gfx_t(ysfx_t *fx) : m_fx(fx) { ysfx_gfx_enter(fx); }
    ~ysfx_scoped_gfx_t() { ysfx_gfx_leave(m_fx); }
    ysfx_t *m_fx = nullptr;
};
#endif

//------------------------------------------------------------------------------
void ysfx_api_init_gfx();
