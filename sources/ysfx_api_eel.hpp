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
#include <string>

typedef void *NSEEL_VMCTX;
class WDL_FastString;

//------------------------------------------------------------------------------
void ysfx_api_init_eel();

//------------------------------------------------------------------------------
void ysfx_eel_string_initvm(NSEEL_VMCTX vm);

//------------------------------------------------------------------------------
class eel_string_context_state;
eel_string_context_state *ysfx_eel_string_context_new();
void ysfx_eel_string_context_free(eel_string_context_state *state);
void ysfx_eel_string_context_update_named_vars(eel_string_context_state *state, NSEEL_VMCTX vm);
YSFX_DEFINE_AUTO_PTR(eel_string_context_state_u, eel_string_context_state, ysfx_eel_string_context_free);

//------------------------------------------------------------------------------
enum { ysfx_string_max_length = 1 << 16 };
bool ysfx_string_access(ysfx_t *fx, ysfx_real id, bool for_write, void (*access)(void *, WDL_FastString &), void *userdata);
bool ysfx_string_get(ysfx_t *fx, ysfx_real id, std::string &txt);
bool ysfx_string_set(ysfx_t *fx, ysfx_real id, const std::string &txt);
