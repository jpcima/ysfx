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
#   define WDL_NO_DEFINE_MINMAX
#   include "WDL/lice/lice.h"
#endif
#include <vector>
#include <memory>
#include <atomic>

#if !defined(YSFX_NO_GFX)
#define GFX_GET_CONTEXT(opaque) (((opaque)) ? ysfx_gfx_get_context((ysfx_t *)(opaque)) : nullptr)

enum {
    ysfx_gfx_max_images = 1024,
};

struct ysfx_gfx_state_t {
    std::atomic<std::thread::id> gfx_thread_id;
    bool framebuffer_dirty = false;
    LICE_WrapperBitmap framebuffer{nullptr, 0, 0, 0, false};
    std::unique_ptr<LICE_MemBitmap> framebuffer_extra;
    std::vector<std::unique_ptr<LICE_IBitmap>> images;
    ysfx_real scale = 0.0;
};

ysfx_gfx_state_t *ysfx_gfx_state_new()
{
    ysfx_gfx_state_u state{new ysfx_gfx_state_t};
    state->images.resize(ysfx_gfx_max_images);
    return state.release();
}

void ysfx_gfx_state_free(ysfx_gfx_state_t *state)
{
    delete state;
}

void ysfx_gfx_state_set_thread(ysfx_gfx_state_t *state, std::thread::id id)
{
    state->gfx_thread_id.store(id, std::memory_order_relaxed);
}

void ysfx_gfx_state_set_bitmap(ysfx_gfx_state_t *state, uint8_t *data, uint32_t w, uint32_t h, uint32_t stride)
{
    if (stride == 0)
        stride = 4 * w;

    bool valid = (stride % 4) == 0;
    if (!valid)
        state->framebuffer = LICE_WrapperBitmap{nullptr, 0, 0, 0, false};
    else
        state->framebuffer = LICE_WrapperBitmap{(LICE_pixel *)data, (int)w, (int)h, (int)(stride / 4), false};
}

void ysfx_gfx_state_set_scale_factor(ysfx_gfx_state_t *state, ysfx_real scale)
{
    state->scale = scale;
}

bool ysfx_gfx_state_is_dirty(ysfx_gfx_state_t *state)
{
    return state->framebuffer_dirty;
}

//------------------------------------------------------------------------------
void ysfx_gfx_enter(ysfx_t *fx, bool doinit)
{
    fx->gfx.mutex.lock();

    if (doinit) {
        if (fx->gfx.must_init.exchange(false, std::memory_order_acquire)) {
            *fx->var.gfx_r = 1.0;
            *fx->var.gfx_g = 1.0;
            *fx->var.gfx_b = 1.0;
            *fx->var.gfx_a = 1.0;
            *fx->var.gfx_a2 = 1.0;
            *fx->var.gfx_dest = -1.0;
            *fx->var.mouse_wheel = 0.0;
            *fx->var.mouse_hwheel = 0.0;
            // NOTE possibly others in the future, check eel_lice.h `resetVarsToStock`
            fx->gfx.ready = true;
        }
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

void ysfx_gfx_prepare(ysfx_t *fx)
{
    ysfx_gfx_state_t *state = ysfx_gfx_get_context(fx);

    state->framebuffer_dirty = false;

    // set variables `gfx_w` and `gfx_h`
    ysfx_real gfx_w = (ysfx_real)state->framebuffer.getWidth();
    ysfx_real gfx_h = (ysfx_real)state->framebuffer.getHeight();
    if (state->scale > 1.0) {
        gfx_w *= state->scale;
        gfx_h *= state->scale;
        *fx->var.gfx_ext_retina = state->scale;
    }
    *fx->var.gfx_w = gfx_w;
    *fx->var.gfx_h = gfx_h;

    // clear the screen
    if (*fx->var.gfx_clear > -1.0) {
        int rgba = (int)*fx->var.gfx_clear;
        LICE_pixel color = LICE_RGBA(rgba & 0xff, (rgba >> 8) & 0xff, (rgba >> 16) & 0xff, 0);
        LICE_Clear(&state->framebuffer, color);
        state->framebuffer_dirty = true;
    }
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
    NSEEL_addfunc_retval("gfx_getchar", 1, NSEEL_PProc_THIS, &ysfx_api_gfx_getchar);
}
