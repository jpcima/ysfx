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
    fx->gfx.mutex.lock();

    if (fx->gfx.must_init.exchange(false, std::memory_order_acquire)) {
        // TODO: perform gfx initializations
        fx->gfx.ready = true;
    }

    ysfx_gfx_state_set_thread(fx->gfx.state.get(), std::this_thread::get_id());
}

void ysfx_gfx_leave(ysfx_t *fx)
{
    fx->gfx.mutex.unlock();
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
#   include "ysfx_api_gfx_lice.hpp"
#else
#   include "ysfx_api_gfx_dummy.hpp"
#endif

//------------------------------------------------------------------------------
void ysfx_api_init_gfx()
{
#if !defined(YSFX_NO_GFX)
    lice_stb_install_loaders();
#endif

    NSEEL_addfunc_retptr("gfx_lineto", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_lineto);
    NSEEL_addfunc_retptr("gfx_lineto", 2, NSEEL_PProc_THIS, &ysfx_api_gfx_lineto2);
    NSEEL_addfunc_retptr("gfx_rectto", 2, NSEEL_PProc_THIS, &ysfx_api_gfx_rectto);
    NSEEL_addfunc_varparm("gfx_rect", 4, NSEEL_PProc_THIS, &ysfx_api_gfx_rect);
    NSEEL_addfunc_varparm("gfx_line", 4, NSEEL_PProc_THIS, &ysfx_api_gfx_line);
    NSEEL_addfunc_varparm("gfx_gradrect", 8, NSEEL_PProc_THIS, &ysfx_api_gfx_gradrect);
    NSEEL_addfunc_varparm("gfx_muladdrect", 7, NSEEL_PProc_THIS, &ysfx_api_gfx_muladdrect);
    NSEEL_addfunc_varparm("gfx_deltablit", 9, NSEEL_PProc_THIS, &ysfx_api_gfx_deltablit);
    NSEEL_addfunc_exparms("gfx_transformblit", 8, NSEEL_PProc_THIS, &ysfx_api_gfx_transformblit);
    NSEEL_addfunc_varparm("gfx_circle", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_circle);
    NSEEL_addfunc_varparm("gfx_triangle", 6, NSEEL_PProc_THIS, &ysfx_api_gfx_triangle);
    NSEEL_addfunc_varparm("gfx_roundrect", 5, NSEEL_PProc_THIS, &ysfx_api_gfx_roundrect);
    NSEEL_addfunc_varparm("gfx_arc", 5, NSEEL_PProc_THIS, &ysfx_api_gfx_arc);
    NSEEL_addfunc_retptr("gfx_blurto", 2, NSEEL_PProc_THIS, &ysfx_api_gfx_blurto);
    NSEEL_addfunc_exparms("gfx_showmenu", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_showmenu);
    NSEEL_addfunc_varparm("gfx_setcursor", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_setcursor);
    NSEEL_addfunc_retptr("gfx_drawnumber", 2, NSEEL_PProc_THIS, &ysfx_api_gfx_drawnumber);
    NSEEL_addfunc_retptr("gfx_drawchar", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_drawchar);
    NSEEL_addfunc_varparm("gfx_drawstr", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_drawstr);
    NSEEL_addfunc_retptr("gfx_measurestr", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_measurestr);
    NSEEL_addfunc_retptr("gfx_measurechar", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_measurechar);
    NSEEL_addfunc_varparm("gfx_printf", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_printf);
    NSEEL_addfunc_retptr("gfx_setpixel", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_setpixel);
    NSEEL_addfunc_retptr("gfx_getpixel", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_getpixel);
    NSEEL_addfunc_retptr("gfx_getimgdim", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_getimgdim);
    NSEEL_addfunc_retval("gfx_setimgdim", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_setimgdim);
    NSEEL_addfunc_retval("gfx_loadimg", 2, NSEEL_PProc_THIS, &ysfx_api_gfx_loadimg);
    NSEEL_addfunc_retptr("gfx_blit", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_blit);
    NSEEL_addfunc_retptr("gfx_blitext", 3, NSEEL_PProc_THIS, &ysfx_api_gfx_blitext);
    NSEEL_addfunc_varparm("gfx_blit", 4, NSEEL_PProc_THIS, &ysfx_api_gfx_blit2);
    NSEEL_addfunc_varparm("gfx_setfont", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_setfont);
    NSEEL_addfunc_varparm("gfx_getfont", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_getfont);
    NSEEL_addfunc_varparm("gfx_set", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_set);
    NSEEL_addfunc_varparm("gfx_getdropfile", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_getdropfile);
    NSEEL_addfunc_varparm("gfx_getsyscol", 0, NSEEL_PProc_THIS, &ysfx_api_gfx_getsyscol);
}
