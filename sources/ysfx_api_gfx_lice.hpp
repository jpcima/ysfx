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
#include "ysfx_api_gfx.hpp"
#include "ysfx_eel_utils.hpp"

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_lineto(void *opaque, EEL_F *xpos, EEL_F *ypos, EEL_F *useaa)
{
    // TODO
    return xpos;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_lineto2(void *opaque, EEL_F *xpos, EEL_F *ypos)
{
    // TODO
    return xpos;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_rectto(void *opaque, EEL_F *xpos, EEL_F *ypos)
{
    // TODO
    return xpos;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_rect(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_line(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_gradrect(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_muladdrect(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
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
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_triangle(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_roundrect(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_arc(void *opaque, INT_PTR np, EEL_F **parms)
{
    // TODO
    return 0;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_blurto(void *opaque, EEL_F *x, EEL_F *y)
{
    // TODO
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
    // TODO
    return r;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_getpixel(void *opaque, EEL_F *r, EEL_F *g, EEL_F *b)
{
    // TODO
    return r;
}

static EEL_F *NSEEL_CGEN_CALL ysfx_api_gfx_getimgdim(void *opaque, EEL_F *img, EEL_F *w, EEL_F *h)
{
    // TODO
    return img;
}

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_setimgdim(void *opaque, EEL_F *img, EEL_F *w, EEL_F *h)
{
    // TODO
    return 0;
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
    // TODO
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
