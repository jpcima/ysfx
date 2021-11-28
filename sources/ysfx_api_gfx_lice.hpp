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
#include "ysfx_api_eel.hpp"
#include "WDL/wdlutf8.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

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

static LICE_IFont *active_font(void *opaque)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = fx->gfx.state.get();

    if (state->font_active < 0 || (uint32_t)state->font_active >= state->fonts.size())
        return nullptr;

    ysfx_gfx_font_t &font = state->fonts[(uint32_t)state->font_active];
    return font.use_fonth ? font.font : nullptr;
}

static bool coords_src_dest_overlap(EEL_F *coords)
{
  if (coords[0] + coords[2] < coords[4]) return false;
  if (coords[0] > coords[4] + coords[6]) return false;
  if (coords[1] + coords[3] < coords[5]) return false;
  if (coords[1] > coords[5] + coords[7]) return false;
  return true;
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

static int current_mode_for_blit(void *opaque, bool isFBsrc)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    const int gmode = (int)(*fx->var.gfx_mode);

    const int sm= (gmode >> 4) & 0xf;

    int mode;
    if (sm > LICE_BLIT_MODE_COPY && sm <= LICE_BLIT_MODE_HSVADJ)
        mode = sm;
    else
        mode = ((gmode & 1) ? LICE_BLIT_MODE_ADD : LICE_BLIT_MODE_COPY);


    if (!isFBsrc && !(gmode & 2))
        mode |= LICE_BLIT_USE_ALPHA;
    if (!(gmode & 4))
        mode |= LICE_BLIT_FILTER_BILINEAR;

    return mode;
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

static int draw_text_with_font(
    LICE_IBitmap *dest, const RECT *rect, LICE_IFont *font, const char *buf, int buflen,
    int fg, int mode, float alpha, int flags, EEL_F *wantYoutput, EEL_F **measureOnly)
{
    if (font) {
        RECT tr = *rect;
        font->SetTextColor(fg);
        font->SetCombineMode(mode, alpha);

        int maxx = 0;
        RECT r = {0, 0, tr.left, 0};
        while (buflen > 0) {
            int thislen = 0;
            while (thislen < buflen && buf[thislen] != '\n') thislen++;
            memset(&r, 0, sizeof(r));
            int lineh = font->DrawText(dest, buf, thislen ? thislen : 1, &r, DT_SINGLELINE|DT_NOPREFIX|DT_CALCRECT);
            if (!measureOnly) {
                r.right += tr.left;
                lineh = font->DrawText(dest, buf, thislen ? thislen : 1, &tr, DT_SINGLELINE|DT_NOPREFIX|flags);
                if (wantYoutput) *wantYoutput = tr.top;
            }
            else {
                if (r.right > maxx) maxx = r.right;
            }
            tr.top += lineh;

            buflen -= thislen + 1;
            buf += thislen + 1;
        }
        if (measureOnly) {
            measureOnly[0][0] = maxx;
            measureOnly[1][0] = tr.top;
        }
        return r.right;
    }
    else
    {
        int xpos = rect->left, ypos = rect->top;
        int x;
        int maxx = 0, maxy = 0;

        LICE_SubBitmap sbm(
            dest, rect->left, rect->top, rect->right-rect->left, rect->bottom-rect->top);

        if (!measureOnly) {
            if (!(flags & DT_NOCLIP)) {
                if (rect->right <= rect->left || rect->bottom <= rect->top) return 0; // invalid clip rect hm

                xpos = ypos = 0;
                dest = &sbm;
            }
            if (flags & (DT_RIGHT|DT_BOTTOM|DT_CENTER|DT_VCENTER)) {
                EEL_F w = 0.0, h = 0.0;
                EEL_F *mo[2] = {&w, &h};
                RECT tr = {0};
                draw_text_with_font(dest, &tr, nullptr, buf, buflen, 0, 0, 0.0f, 0, nullptr, mo);

                if (flags & DT_RIGHT) xpos += (rect->right-rect->left) - (int)std::floor(w);
                else if (flags & DT_CENTER) xpos += (rect->right-rect->left) / 2 - (int)std::floor(w * (EEL_F).5);

                if (flags & DT_BOTTOM) ypos += (rect->bottom-rect->top) - (int)std::floor(h);
                else if (flags & DT_VCENTER) ypos += (rect->bottom-rect->top) / 2 - (int)std::floor(h * (EEL_F).5);
            }
        }
        const int sxpos = xpos;

        for (x = 0; x < buflen; x++) {
            switch (buf[x]) {
            case '\n':
                ypos += 8;
            case '\r':
                xpos = sxpos;
                break;
            case ' ': xpos += 8; break;
            case '\t': xpos += 8 * 5; break;
            default:
                if (!measureOnly) LICE_DrawChar(dest, xpos, ypos, buf[x], fg, alpha, mode);
                xpos += 8;
                if (xpos > maxx) maxx = xpos;
                maxy = ypos + 8;
                break;
            }
        }
        if (measureOnly) {
            measureOnly[0][0] = maxx;
            measureOnly[1][0] = maxy;
        }
        else {
            if (wantYoutput) *wantYoutput = ypos;
        }
        return xpos;
    }
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_drawnumber(void *opaque, EEL_F *n, EEL_F *nd)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return n;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_drawnumber");
    if (!dest)
        return n;

    set_image_dirty(opaque, dest);

    char buf[512];
    int a = (int)(*nd + (EEL_F)0.5);
    if (a < 0) a = 0;
    if (a > 16) a = 16;
    snprintf(buf, sizeof(buf), "%.*f", a, (EEL_F)*n);

    RECT r = {(int)std::floor(*fx->var.gfx_x), (int)std::floor(*fx->var.gfx_y), 0, 0};
    *fx->var.gfx_x = draw_text_with_font(
        dest, &r, active_font(opaque), buf, (int)std::strlen(buf),
        current_color(opaque), current_mode(opaque), (float)*fx->var.gfx_a, DT_NOCLIP, nullptr, nullptr);

    return n;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_drawchar(void *opaque, EEL_F *n)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return n;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_drawchar");
    if (!dest)
        return n;

    set_image_dirty(opaque, dest);

    int a = (int)(*n + (EEL_F)0.5);
    if (a == '\r' || a == '\n') a = ' ';

    char buf[32];
    const int buflen = WDL_MakeUTFChar(buf, a, sizeof(buf));

    RECT r = {(int)std::floor(*fx->var.gfx_x), (int)std::floor(*fx->var.gfx_y), 0, 0};
    *fx->var.gfx_x = draw_text_with_font(
        dest, &r, active_font(opaque), buf, buflen,
        current_color(opaque), current_mode(opaque), (float)*fx->var.gfx_a, DT_NOCLIP, nullptr, nullptr);

    return n;
}

static void ysfx_gfx_api_dostr(void *opaque, EEL_F **parms, INT_PTR nparms, int formatmode)// formatmode=1 for format, 2 for purely measure no format
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return;

    INT_PTR nfmtparms = nparms - 1;
    EEL_F **fmtparms = parms + 1;
    const char *funcname =
        formatmode == 1 ? "gfx_printf" :
        formatmode == 2 ? "gfx_measurestr" :
        formatmode == 3 ? "gfx_measurechar" : "gfx_drawstr";

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, funcname);
    if (!dest)
        return;

    std::string fs;
    char buf[4096];
    int s_len = 0;

    const char *s = nullptr;
    if (formatmode == 3) {
        s_len = WDL_MakeUTFChar(buf, (int)parms[0][0], sizeof(buf));
        s = buf;
    }
    else {
        if (ysfx_string_get(fx, parms[0][0], fs))
            s = fs.c_str();
#ifdef EEL_STRING_DEBUGOUT
        if (!s) EEL_STRING_DEBUGOUT("gfx_%s: invalid string identifier %f", funcname, parms[0][0]);
#endif
        if (!s) {
            s = "<bad string>";
            s_len = 12;
        }
        else if (formatmode == 1) {
            extern int eel_format_strings(void *, const char *s, const char *ep, char *, int, int, EEL_F **);
            s_len = eel_format_strings(opaque, s, (s + fs.size()), buf, sizeof(buf), nfmtparms, fmtparms);
            if (s_len < 1) return;
            s = buf;
        }
        else {
            s_len = (int)fs.size();
        }
    }

    if (s_len) {
        set_image_dirty(opaque, dest);
        if (formatmode >= 2) {
            if (nfmtparms == 2) {
                RECT r = {0,0,0,0};
                draw_text_with_font(
                    dest, &r, active_font(opaque), s, s_len,
                    current_color(opaque), current_mode(opaque), (float)*fx->var.gfx_a, 0, nullptr, fmtparms);
            }
        }
        else {
            RECT r = {(int)std::floor(*fx->var.gfx_x), (int)std::floor(*fx->var.gfx_y), 0, 0};
            int flags = DT_NOCLIP;
            if (formatmode == 0 && nparms >= 4) {
                flags = (int)*parms[1];
                flags &= (DT_CENTER|DT_RIGHT|DT_VCENTER|DT_BOTTOM|DT_NOCLIP);
                r.right = (int)*parms[2];
                r.bottom = (int)*parms[3];
            }
            *fx->var.gfx_x = draw_text_with_font(
                dest, &r, active_font(opaque), s, s_len,
                current_color(opaque), current_mode(opaque), (float)*fx->var.gfx_a, flags, fx->var.gfx_y, nullptr);
        }
    }
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_drawstr(void *opaque, INT_PTR nparms, EEL_F **parms)
{
    ysfx_gfx_api_dostr(opaque, parms, nparms, 0);
    return parms[0][0];
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_measurestr(void *opaque, EEL_F *str, EEL_F *xOut, EEL_F *yOut)
{
    EEL_F *p[3] = {str, xOut, yOut};
    ysfx_gfx_api_dostr(opaque, p, 3, 2);
    return str;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_measurechar(void *opaque, EEL_F *str, EEL_F *xOut, EEL_F *yOut)
{
    EEL_F *p[3] = {str, xOut, yOut};
    ysfx_gfx_api_dostr(opaque, p, 3, 3);
    return str;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_printf(void *opaque, INT_PTR nparms, EEL_F **parms)
{
    if (nparms < 1)
        return 0;

    EEL_F v = **parms;
    ysfx_gfx_api_dostr(opaque, parms, nparms, 1);
    return v;
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

    return rv ? 1 : 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_loadimg(void *opaque, EEL_F *img, EEL_F *fr)
{
    // TODO
    return 0;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_blit(void *opaque, EEL_F *img, EEL_F *scale, EEL_F *rotate)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return img;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_blit");
    if (!dest)
        return img;

    LICE_IBitmap *bm = image_for_index(opaque, *img, "gfx_blit:src");
    if (!bm)
        return img;

    set_image_dirty(opaque, dest);
    const bool isFromFB = bm == &state->framebuffer;

    int bmw = bm->getWidth();
    int bmh = bm->getHeight();
    if (std::fabs(*rotate) > (EEL_F)0.000000001) {
        LICE_RotatedBlit(dest, bm, (int)*fx->var.gfx_x, (int)*fx->var.gfx_y, (int)(bmw * scale[0]),(int)(bmh * scale[0]), 0.0f, 0.0f, (float)bmw, (float)bmh, (float)rotate[0], true, (float)*fx->var.gfx_a, current_mode_for_blit(opaque, isFromFB),
                         0.0f, 0.0f);
    }
    else {
        LICE_ScaledBlit(dest, bm, (int)*fx->var.gfx_x, (int)*fx->var.gfx_y,(int) (bmw * scale[0]),(int) (bmh * scale[0]), 0.0f, 0.0f, (float)bmw, (float)bmh, (float)*fx->var.gfx_a, current_mode_for_blit(opaque, isFromFB));
    }

    return img;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_blitext(void *opaque, EEL_F *img, EEL_F *coordidx, EEL_F *rotate)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return img;

    EEL_F coords[10];
    {
        ysfx_eel_ram_reader rr{fx->vm.get(), (int64_t)*coordidx};
        for (int i = 0; i < 10; ++i)
            coords[i] = rr.read_next();
    }

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_blitext");
    if (!dest)
        return img;

    LICE_IBitmap *bm = image_for_index(opaque, *img, "gfx_blitext:src");
    if (!bm)
        return img;

    set_image_dirty(opaque, dest);
    const bool isFromFB = bm == &state->framebuffer;

    int bmw = bm->getWidth();
    int bmh = bm->getHeight();

    if (bm == dest && coords_src_dest_overlap(coords)) {
        if (!state->framebuffer_extra)
            state->framebuffer_extra.reset(new LICE_MemBitmap(bmw, bmh));
        if (state->framebuffer_extra) {
            bm = state->framebuffer_extra.get();
            bm->resize(bmw, bmh);
            LICE_ScaledBlit(bm, dest, // copy the source portion
                            (int)coords[0], (int)coords[1], (int)coords[2], (int)coords[3],
                            (float)coords[0], (float)coords[1], (float)coords[2], (float)coords[3],
                            1.0f, LICE_BLIT_MODE_COPY);
        }
    }

    EEL_F angle = *rotate;
    if (std::fabs(angle) > (EEL_F)0.000000001) {
        LICE_RotatedBlit(dest, bm, (int)coords[4], (int)coords[5], (int)coords[6], (int)coords[7],
                         (float)coords[0], (float)coords[1], (float)coords[2], (float)coords[3], (float)angle,
                         true, (float)*fx->var.gfx_a, current_mode_for_blit(opaque, isFromFB),
                         (float)coords[8], (float)coords[9]);
    }
    else {
        LICE_ScaledBlit(dest, bm, (int)coords[4], (int)coords[5], (int)coords[6], (int)coords[7],
                        (float)coords[0], (float)coords[1], (float)coords[2], (float)coords[3], (float)*fx->var.gfx_a, current_mode_for_blit(opaque, isFromFB));
    }

    return img;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_blitext2(void *opaque, INT_PTR np, EEL_F **parms, int blitmode)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_blitext2");
    if (!dest)
        return 0;

    LICE_IBitmap *bm = image_for_index(opaque, parms[0][0], "gfx_blitext2:src");
    if (!bm)
        return 0;

    const int bmw = bm->getWidth();
    const int bmh = bm->getHeight();

    // 0=img, 1=scale, 2=rotate
    EEL_F coords[8];
    const EEL_F sc = (blitmode == 0) ? parms[1][0] : 1.0;
    const EEL_F angle = (blitmode == 0) ? parms[2][0] : 0.0;
    if (blitmode == 0) {
        parms += 2;
        np -= 2;
    }

    coords[0] = (np > 1) ? parms[1][0] : 0;
    coords[1] = (np > 2) ? parms[2][0] : 0;
    coords[2] = (np > 3) ? parms[3][0] : bmw;
    coords[3] = (np > 4) ? parms[4][0] : bmh;
    coords[4] = (np > 5) ? parms[5][0] : *fx->var.gfx_x;
    coords[5] = (np > 6) ? parms[6][0] : *fx->var.gfx_y;
    coords[6] = (np > 7) ? parms[7][0] : coords[2]*sc;
    coords[7] = (np > 8) ? parms[8][0] : coords[3]*sc;

    const bool isFromFB = bm == &state->framebuffer;
    set_image_dirty(opaque, dest);

    if (bm == dest && coords_src_dest_overlap(coords)) {
        if (!state->framebuffer_extra)
            state->framebuffer_extra.reset(new LICE_MemBitmap(bmw, bmh));
        if (state->framebuffer_extra) {
            bm = state->framebuffer_extra.get();
            bm->resize(bmw, bmh);
            LICE_ScaledBlit(bm, dest, // copy the source portion
                            (int)coords[0], (int)coords[1], (int)coords[2], (int)coords[3],
                            (float)coords[0], (float)coords[1], (float)coords[2], (float)coords[3],
                            1.0f, LICE_BLIT_MODE_COPY);
        }
    }

    if (blitmode == 1) {
        LICE_DeltaBlit(dest, bm, (int)coords[4], (int)coords[5], (int)coords[6], (int)coords[7],
                       (float)coords[0], (float)coords[1], (float)coords[2], (float)coords[3],
                       (np > 9) ? (float)parms[9][0] : 1, // dsdx
                       (np > 10) ? (float)parms[10][0] : 0, // dtdx
                       (np > 11) ? (float)parms[11][0] : 0, // dsdy
                       (np > 12) ? (float)parms[12][0] : 1, // dtdy
                       (np > 13) ? (float)parms[13][0] : 0, // dsdxdy
                       (np > 14) ? (float)parms[14][0] : 0, // dtdxdy
                       (np <= 15 || parms[15][0] > (EEL_F)0.5), (float)*fx->var.gfx_a, current_mode_for_blit(opaque, isFromFB));
    }
    else if (std::fabs(angle) > (EEL_F)0.000000001) {
        LICE_RotatedBlit(dest, bm, (int)coords[4], (int)coords[5], (int)coords[6], (int)coords[7],
                         (float)coords[0], (float)coords[1], (float)coords[2], (float)coords[3],
                         (float)angle, true, (float)*fx->var.gfx_a, current_mode_for_blit(opaque, isFromFB),
                         (np > 9) ? (float)parms[9][0] : 0,
                         (np > 10) ? (float)parms[10][0] : 0);
    }
    else {
        LICE_ScaledBlit(dest, bm, (int)coords[4], (int)coords[5], (int)coords[6], (int)coords[7],
                        (float)coords[0], (float)coords[1], (float)coords[2], (float)coords[3], (float)*fx->var.gfx_a, current_mode_for_blit(opaque, isFromFB));
    }

    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_blit2(void *opaque, INT_PTR np, EEL_F **parms)
{
    if (np >= 3) {
        ysfx_api_gfx_blitext2(opaque, np, parms, 0);
        return *parms[0];
    }
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_deltablit(void *opaque, INT_PTR np, EEL_F **parms)
{
    ysfx_api_gfx_blitext2(opaque, np, parms, 1);
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_transformblit(void *opaque, INT_PTR np, EEL_F **parms)
{
    ysfx_t *fx = (ysfx_t *)opaque;
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    // NOTE(jpc) enforced limit to 64, as indicated by specification
    const int divw = std::min(64, (int)(parms[5][0] + (EEL_F)0.5));
    const int divh = std::min(64, (int)(parms[6][0] + (EEL_F)0.5));
    if (divw < 1 || divh < 1)
        return 0;
    const int sz = divw * divh * 2;

    double *d;

    //
    double d_small[128];
    std::unique_ptr<double[]> d_large;
    if (sz <= 128)
        d = d_small;
    else
        d_large.reset((d = new double[sz]));

    {
        ysfx_eel_ram_reader rr{fx->vm.get(), (int)(parms[7][0] + (EEL_F)0.5)};
        for (int i = 0; i < sz; ++i)
            d[i] = rr.read_next();
    }

    //
    LICE_IBitmap *dest = image_for_index(opaque, *fx->var.gfx_dest, "gfx_transformblit");
    if (!dest)
        return 0;

    LICE_IBitmap *bm = image_for_index(opaque, parms[0][0], "gfx_transformblit:src");
    if (!bm)
        return 0;

    const int bmw = bm->getWidth();
    const int bmh = bm->getHeight();

    const bool isFromFB = bm == &state->framebuffer;

    set_image_dirty(opaque, dest);

    if (bm == dest) {
        if (!state->framebuffer_extra)
            state->framebuffer_extra.reset(new LICE_MemBitmap(bmw, bmh));
        if (state->framebuffer_extra) {
            bm = state->framebuffer_extra.get();
            bm->resize(bmw, bmh);
            LICE_ScaledBlit(bm, dest, // copy the entire image
                            0, 0, bmw, bmh,
                            0.0f, 0.0f, (float)bmw, (float)bmh,
                            1.0f, LICE_BLIT_MODE_COPY);
        }
    }
    LICE_TransformBlit2(dest, bm, (int)std::floor(parms[1][0]), (int)std::floor(parms[2][0]), (int)std::floor(parms[3][0]), (int)std::floor(parms[4][0]), d, divw, divh, (float)*fx->var.gfx_a, current_mode_for_blit(opaque, isFromFB));

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

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_getchar(void *opaque, EEL_F *p)
{
    ysfx_gfx_state_t *state = GFX_GET_CONTEXT(opaque);
    if (!state)
        return 0;

    if (*p >= 1/*2*/) { // NOTE(jpc) this is 2.0 originally, which seems wrong
        if (*p == 65536) {
            // TODO implement window flags
            return 0;
        }

        // current key down status
        uint32_t key = (uint32_t)*p;
        uint32_t key_id;
        if (translate_special_key(key, key))
            key_id = key;
        else if (key < 256)
            key_id = ysfx::latin1_tolower(key);
        else // support the Latin-1 character set only
            return 0;
        return (EEL_F)(state->keys_pressed.find(key_id) != state->keys_pressed.end());
    }

    if (!state->input_queue.empty()) {
        uint32_t key = state->input_queue.front();
        state->input_queue.pop();
        return (EEL_F)key;
    }

    return 0;
}
