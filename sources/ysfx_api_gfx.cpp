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

#include "ysfx_api_gfx.hpp"
#include "ysfx_eel_utils.hpp"

static EEL_F NSEEL_CGEN_CALL ysfx_api_gfx_stub(void *opaque, INT_PTR np, EEL_F **parms)
{
    (void)opaque;
    (void)np;
    (void)parms;
    return 0;
}

void ysfx_api_init_gfx()
{
    //TODO implement me
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
}
