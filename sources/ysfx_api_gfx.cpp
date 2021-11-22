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
#include "ysfx_api_gfx.hpp"
#include "ysfx_eel_utils.hpp"
#if !defined(YSFX_NO_GFX)
#   include "lice_stb/lice_stb_loaders.hpp"
#endif
#include <atomic>

#if !defined(YSFX_NO_GFX)
#define GFX_GET_CONTEXT(opaque) (((opaque)) ? ysfx_gfx_get_context((ysfx_t *)(opaque)) : nullptr)

struct ysfx_gfx_state_t {
    std::atomic<std::thread::id> gfx_thread_id;
};

ysfx_gfx_state_t *ysfx_gfx_state_new()
{
    return new ysfx_gfx_state_t;
}

void ysfx_gfx_state_free(ysfx_gfx_state_t *state)
{
    delete state;
}

void ysfx_gfx_state_set_thread(ysfx_gfx_state_t *state, std::thread::id id)
{
    state->gfx_thread_id.store(id, std::memory_order_relaxed);
}

//------------------------------------------------------------------------------
void ysfx_gfx_enter(ysfx_t *fx)
{
    ysfx_gfx_state_set_thread(fx->gfx.state.get(), std::this_thread::get_id());
}

ysfx_gfx_state_t *ysfx_gfx_get_context(ysfx_t *fx)
{
    // NOTE: make sure that this will be used from the @gfx thread only
    if (!fx)
        return nullptr;
    ysfx_gfx_state_t *state = fx->gfx.state.get();
    if (state->gfx_thread_id.load(std::memory_order_relaxed) != std::this_thread::get_id())
        return nullptr;
    return state;
}
#endif

//------------------------------------------------------------------------------
#if !defined(YSFX_NO_GFX)
    // TODO: implement gfx API
#endif

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_stub(void *opaque, INT_PTR np, EEL_F **parms)
{
    (void)opaque;
    (void)np;
    (void)parms;
    return 0;
}

void ysfx_api_init_gfx()
{
#if !defined(YSFX_NO_GFX)
    lice_stb_install_loaders();
#endif

#if !defined(YSFX_NO_GFX)
    // TODO: implement these
    NSEEL_addfunc_varparm("gfx_set", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_lineto", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_line", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_rectto", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_rect", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_setpixel", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_getpixel", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_drawnumber", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_drawchar", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_drawstr", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_measurestr", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_setfont", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_getfont", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_printf", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_blurto", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_blit", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_blitext", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_getimgdim", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_setimgdim", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_loadimg", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_gradrect", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_muladdrect", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_deltablit", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_transformblit", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_circle", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_roundrect", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_arc", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_triangle", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_getchar", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_showmenu", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_setcursor", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
#else
    // stubs
    NSEEL_addfunc_varparm("gfx_set", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_lineto", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_line", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_rectto", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_rect", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_setpixel", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_getpixel", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_drawnumber", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_drawchar", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_drawstr", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_measurestr", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_setfont", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_getfont", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_printf", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_blurto", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_blit", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_blitext", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_getimgdim", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_setimgdim", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_loadimg", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_gradrect", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_muladdrect", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_deltablit", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_transformblit", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_circle", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_roundrect", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_arc", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_triangle", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_getchar", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_showmenu", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
    NSEEL_addfunc_varparm("gfx_setcursor", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_stub);
#endif
}
