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
#include <juce_core/juce_core.h>
#include <memory>

struct YsfxSliderInfo : public std::enable_shared_from_this<YsfxSliderInfo> {
    bool exists = false;
    juce::String name;
    ysfx_slider_range_t range{};
    bool isEnum = false;
    bool isPath = false;
    juce::StringArray enumNames;
    //
    using Ptr = std::shared_ptr<YsfxSliderInfo>;
    static YsfxSliderInfo::Ptr none();
    static YsfxSliderInfo::Ptr extractFrom(ysfx_t *fx, uint32_t index);
};

struct YsfxInfo : public std::enable_shared_from_this<YsfxInfo> {
    juce::String name;
    juce::String path;
    bool isLoaded = false;
    bool isCompiled = false;
    YsfxSliderInfo::Ptr sliders[ysfx_max_sliders];
    //
    using Ptr = std::shared_ptr<YsfxInfo>;
    YsfxInfo();
    static YsfxInfo::Ptr extractFrom(ysfx_t *fx);
};
