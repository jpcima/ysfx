//
// This file is based in part on modified source code from `WDL/eel2/eel_lice.h`.
// The zlib license from the WDL applies to this source file.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2021 and later Jean Pierre Cimalando
// Copyright (C) 2005 and later Cockos Incorporated
//
//
// Portions copyright other contributors, see each source file for more information
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// SPDX-License-Identifier: Zlib
//

#pragma once
#include <algorithm>
#include <cmath>

// help clangd to figure things out
#if defined(__CLANGD__)
#   include "ysfx_api_gfx.cpp"
#endif

//------------------------------------------------------------------------------
static void set_image_dirty(void *opaque, LICE_IBitmap *bm)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = fx->gfx.state.get();

    if (bm == &state->framebuffer)
        state->framebuffer_dirty = true;
}

static LICE_IBitmap *image_for_index(void *opaque, EEL_F idx, const char *callername)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = fx->gfx.state.get();
    if (idx > -2) {
        if (idx < 0)
            return &state->framebuffer;

        const int a = (int)idx;
        if (a >= 0 && (size_t)a < state->images.size())
            return state->images[(size_t)a].get();
    }
    return nullptr;
}

static int current_mode(void *opaque)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    const int gmode = (int)(*fx->var.gfx_mode);

    const int sm = (gmode >> 4) & 0xf;
    if (sm > LICE_BLIT_MODE_COPY && sm <= LICE_BLIT_MODE_HSVADJ)
        return sm;

    return (gmode & 1) ? LICE_BLIT_MODE_ADD : LICE_BLIT_MODE_COPY;
}

static LICE_pixel current_color(void *opaque)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    int red = (int)(*fx->var.gfx_r * 255);
    int green = (int)(*fx->var.gfx_g * 255);
    int blue = (int)(*fx->var.gfx_b * 255);
    int a2 = (int)(*fx->var.gfx_a2 * 255);
    if (red < 0) red = 0; else if (red > 255) red = 255;
    if (green < 0) green = 0; else if (green > 255) green = 255;
    if (blue < 0) blue = 0; else if (blue > 255) blue = 255;
    if (a2 < 0) a2 = 0; else if (a2 > 255) a2 = 255;
    return LICE_RGBA(red, green, blue, a2);
}

//------------------------------------------------------------------------------
static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_lineto(void *opaque, EEL_F *xpos, EEL_F *ypos, EEL_F *useaa)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return xpos;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_lineto");
    if (!dest)
        return xpos;

    int x1 = (int)std::floor(*xpos);
    int y1 = (int)std::floor(*ypos);
    int x2 = (int)std::floor(*fx->var.gfx_x);
    int y2 = (int)std::floor(*fx->var.gfx_y);
    if (LICE_ClipLine(&x1, &y1, &x2, &y2, 0, 0, dest->getWidth(), dest->getHeight())) {
        set_image_dirty(opaque, dest);
        LICE_Line(dest, x1, y1, x2, y2, current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque), *useaa > (EEL_F)0.5);
    }
    *fx->var.gfx_x = *xpos;
    *fx->var.gfx_y = *ypos;

    return xpos;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_lineto2(void *opaque, EEL_F *xpos, EEL_F *ypos)
{
    EEL_F useaa = 1;
    return ysfx_api_gfx_lineto(opaque, xpos, ypos, &useaa);
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_rectto(void *opaque, EEL_F *xpos, EEL_F *ypos)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return xpos;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_rectto");
    if (!dest)
        return xpos;

    EEL_F x1 = *xpos;
    EEL_F y1 = *ypos;
    EEL_F x2 = *fx->var.gfx_x;
    EEL_F y2 = *fx->var.gfx_y;
    if (x2 < x1) { x1 = x2; x2 = *xpos; }
    if (y2 < y1) { y1 = y2; y2 = *ypos; }

    if (x2 - x1 > 0.5 && y2 - y1 > 0.5) {
        set_image_dirty(opaque, dest);
        LICE_FillRect(dest, (int)x1, (int)y1, (int)(x2 - x1), (int)(y2 - y1), current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque));
    }
    *fx->var.gfx_x = *xpos;
    *fx->var.gfx_y = *ypos;

    return xpos;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_rect(void *opaque, INT_PTR np, EEL_F **parms)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_rect");
    if (!dest)
        return 0;

    int x1 = (int)std::floor(parms[0][0]);
    int y1 = (int)std::floor(parms[1][0]);
    int w = (int)std::floor(parms[2][0]);
    int h = (int)std::floor(parms[3][0]);
    int filled = (np < 5 || parms[4][0] > (EEL_F)0.5);

    if (w > 0 && h > 0) {
        set_image_dirty(opaque, dest);
        if (filled) LICE_FillRect(dest, x1, y1, w, h, current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque));
        else LICE_DrawRect(dest, x1, y1, w - 1, h - 1, current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque));
    }

    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_line(void *opaque, INT_PTR np, EEL_F **parms)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_line");
    if (!dest)
        return 0;

    int x1 = (int)std::floor(parms[0][0]);
    int y1 = (int)std::floor(parms[1][0]);
    int x2 = (int)std::floor(parms[2][0]);
    int y2 = (int)std::floor(parms[3][0]);
    if (LICE_ClipLine(&x1, &y1, &x2, &y2, 0, 0, dest->getWidth(),dest->getHeight())) {
        set_image_dirty(opaque, dest);
        LICE_Line(dest, x1, y1, x2, y2, current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque), np < 5 || parms[4][0] > (EEL_F)0.5);
    }

    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_grad_or_muladd_rect(void *opaque, int whichmode, INT_PTR np, EEL_F **parms)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, (whichmode == 0) ? "gfx_gradrect" : "gfx_muladdrect");
    if (!dest)
        return 0;

    const int x1 = (int)std::floor(parms[0][0]);
    const int y1 = (int)std::floor(parms[1][0]);
    const int w = (int)std::floor(parms[2][0]);
    const int h = (int)std::floor(parms[3][0]);

    if (w > 0 && h > 0) {
        set_image_dirty(opaque, dest);
        if (whichmode == 0 && np > 7) {
            LICE_GradRect(dest, x1, y1, w, h, (float)parms[4][0], (float)parms[5][0], (float)parms[6][0], (float)parms[7][0],
                          (np > 8) ? (float)parms[8][0] : 0, (np > 9) ? (float)parms[9][0] : 0, (np > 10) ? (float)parms[10][0] : 0, (np > 11) ? (float)parms[11][0] : 0,
                          (np > 12) ? (float)parms[12][0] : 0, (np > 13) ? (float)parms[13][0] : 0,  (np > 14) ? (float)parms[14][0] : 0, (np > 15) ? (float)parms[15][0] : 0,
                          current_mode(opaque));
        }
        else if (whichmode == 1 && np > 6) {
            const EEL_F sc = 255;
            LICE_MultiplyAddRect(dest, x1, y1, w, h, (float)parms[4][0], (float)parms[5][0], (float)parms[6][0], (np > 7) ? (float)parms[7][0] : 1,
                                 (float)((np > 8) ? sc * parms[8][0] : 0), (float)((np > 9) ? sc * parms[9][0] : 0),  (float)((np > 10) ? sc * parms[10][0] : 0), (float)((np > 11) ? sc * parms[11][0] : 0));
        }
    }

    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_gradrect(void *opaque, INT_PTR np, EEL_F **parms)
{
    return ysfx_api_gfx_grad_or_muladd_rect(opaque, 0, np, parms);
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_muladdrect(void *opaque, INT_PTR np, EEL_F **parms)
{
    return ysfx_api_gfx_grad_or_muladd_rect(opaque, 1, np, parms);
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_deltablit(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_transformblit(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_circle(void *opaque, INT_PTR np, EEL_F **parms)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    float x = (float)parms[0][0];
    float y = (float)parms[1][0];
    float r = (float)parms[2][0];

    bool aa = true, fill = false;
    if (np > 3)
        fill = parms[3][0] > 0.5;
    if (np > 4)
        aa = parms[4][0] > 0.5;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_circle");
    if (!dest)
        return 0;

    set_image_dirty(opaque, dest);
    if (fill)
        LICE_FillCircle(dest, x, y, r, current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque), aa);
    else
        LICE_Circle(dest, x, y, r, current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque), aa);

    return 0.0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_triangle(void *opaque, INT_PTR np, EEL_F **parms)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_triangle");
    if (np >= 6) {
        np &= ~1;
        set_image_dirty(opaque, dest);
        if (np == 6) {
            LICE_FillTriangle(dest, (int)parms[0][0], (int)parms[1][0], (int)parms[2][0], (int)parms[3][0],
                              (int)parms[4][0], (int)parms[5][0], current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque));
        }
        else {
            const int maxpt = 512;
            const int n = std::min((int)(np / 2), maxpt);
            int i, rdi = 0;
            int x[maxpt], y[maxpt];
            for (i = 0; i < n; i++)
            {
                x[i] = (int)parms[rdi++][0];
                y[i] = (int)parms[rdi++][0];
            }

            LICE_FillConvexPolygon(dest, x, y, n, current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque));
        }
    }

    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_roundrect(void *opaque, INT_PTR np, EEL_F **parms)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_roundrect");
    if (!dest)
        return 0;

    const bool aa = np <= 5 || parms[5][0] > (EEL_F)0.5;

    if (parms[2][0] > 0 && parms[3][0] > 0) {
        set_image_dirty(opaque, dest);
        LICE_RoundRect(dest, (float)parms[0][0], (float)parms[1][0], (float)parms[2][0], (float)parms[3][0], (int)parms[4][0], current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque), aa);
    }

    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_arc(void *opaque, INT_PTR np, EEL_F **parms)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_arc");
    if (!dest)
        return 0;

    const bool aa = np <= 5 || parms[5][0] > (EEL_F)0.5;

    set_image_dirty(opaque, dest);
    LICE_Arc(dest, (float)parms[0][0], (float)parms[1][0], (float)parms[2][0], (float)parms[3][0], (float)parms[4][0], current_color(opaque), (float)*fx->var.gfx_a, current_mode(opaque), aa);

    return 0;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_blurto(void *opaque, EEL_F *x, EEL_F *y)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return x;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_blurto");
    if (!dest)
        return x;

    set_image_dirty(opaque, dest);

    int srcx = (int)*x;
    int srcy = (int)*y;
    int srcw = (int)(*fx->var.gfx_x - *x);
    int srch = (int)(*fx->var.gfx_y - *y);
    if (srch < 0) { srch = -srch; srcy = (int)*fx->var.gfx_y; }
    if (srcw < 0) { srcw = -srcw; srcx = (int)*fx->var.gfx_x; }
    LICE_Blur(dest, dest, srcx, srcy, srcx, srcy, srcw, srch);
    *fx->var.gfx_x = *x;
    *fx->var.gfx_y = *y;

    return x;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_showmenu(void *opaque, INT_PTR nparms, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_setcursor(void *opaque,  INT_PTR nparms, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_drawnumber(void *opaque, EEL_F *n, EEL_F *nd)
{
    // TODO
    return n;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_drawchar(void *opaque, EEL_F *n)
{
    // TODO
    return n;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_drawstr(void *opaque, INT_PTR nparms, EEL_F **parms)
{
    // TODO
    return 0;
}


static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_measurestr(void *opaque, EEL_F *str, EEL_F *xOut, EEL_F *yOut)
{
    // TODO
    return str;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_measurechar(void *opaque, EEL_F *str, EEL_F *xOut, EEL_F *yOut)
{
    // TODO
    return str;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_printf(void *opaque, INT_PTR nparms, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_setpixel(void *opaque, EEL_F *r, EEL_F *g, EEL_F *b)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return r;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest,"gfx_setpixel");
    if (!dest)
        return r;

    int red = (int)(*r * 255);
    int green = (int)(*g * 255);
    int blue = (int)(*b * 255);
    if (red < 0) red = 0; else if (red > 255) red = 255;
    if (green < 0) green = 0; else if (green > 255) green = 255;
    if (blue < 0) blue = 0; else if (blue > 255) blue = 255;

    set_image_dirty(opaque, dest);
    LICE_PutPixel(dest, (int)*fx->var.gfx_x, (int)*fx->var.gfx_y, LICE_RGBA(red, green, blue, 255), (float)*fx->var.gfx_a, current_mode(opaque));

    return r;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_getpixel(void *opaque, EEL_F *r, EEL_F *g, EEL_F *b)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return r;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_getpixel");
    if (!dest)
        return r;

    int ret = LICE_GetPixel(dest, (int)*fx->var.gfx_x, (int)*fx->var.gfx_y);

    *r = LICE_GETR(ret) / (EEL_F)255;
    *g = LICE_GETG(ret) / (EEL_F)255;
    *b = LICE_GETB(ret) / (EEL_F)255;

    return r;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_getimgdim(void *opaque, EEL_F *img, EEL_F *w, EEL_F *h)
{
    *w = 0;
    *h = 0;

    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return img;

    LICE_IBitmap *bm = image_for_index(opaque, *img, "gfx_getimgdim");
    if (bm) {
        *w = bm->getWidth();
        *h = bm->getHeight();
    }

    return img;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_setimgdim(void *opaque, EEL_F *img, EEL_F *w, EEL_F *h)
{
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    int rv = 0;

    int use_w = (int)*w;
    int use_h = (int)*h;
    if (use_w < 1 || use_h < 1) use_w = use_h = 0;
    if (use_w > 8192) use_w = 8192;
    if (use_h > 8192) use_h = 8192;

    LICE_IBitmap *bm = nullptr;
    if (*img >= 0 && (size_t)*img < state->images.size()) {
        bm = state->images[(size_t)*img].get();
        if (!bm) {
            bm = new LICE_SysBitmap(use_w, use_h);
            state->images[(size_t)*img].reset(bm);
            rv = !!bm;
        }
        else {
            rv = bm->resize(use_w, use_h);
        }
    }

    return rv?1.0:0.0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_loadimg(void *opaque, EEL_F *img, EEL_F *fr)
{
    // TODO
    return 0;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_blit(void *opaque, EEL_F *img, EEL_F *scale, EEL_F *rotate)
{
    // TODO
    return img;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_blitext(void *opaque, EEL_F *img, EEL_F *coordidx, EEL_F *rotate)
{
    // TODO
    return img;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_blit2(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_setfont(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_getfont(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_set(void *opaque, INT_PTR np, EEL_F **parms)
{
    if (np < 1)
        return 0;

    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    *fx->var.gfx_r = parms[0][0];
    *fx->var.gfx_g = (np > 1) ? parms[1][0] : parms[0][0];
    *fx->var.gfx_b = (np > 2) ? parms[2][0] : parms[0][0];
    *fx->var.gfx_a = (np > 3) ? parms[3][0] : 1;
    *fx->var.gfx_mode = (np > 4) ? parms[4][0] : 0;
    if (np > 5) *fx->var.gfx_dest = parms[5][0];
    *fx->var.gfx_a2 = (np > 6) ? parms[6][0] : 1;

    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_getdropfile(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_getsyscol(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}
