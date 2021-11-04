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

#include "info.h"

YsfxSliderInfo::Ptr YsfxSliderInfo::none()
{
    static YsfxSliderInfo::Ptr info{new YsfxSliderInfo};
    return info;
}

YsfxSliderInfo::Ptr YsfxSliderInfo::extractFrom(ysfx_t *fx, uint32_t index)
{
    if (!ysfx_slider_exists(fx, index))
        return none();

    YsfxSliderInfo::Ptr slider{new YsfxSliderInfo};
    slider->exists = true;
    slider->name = juce::CharPointer_UTF8(ysfx_slider_get_name(fx, index));
    ysfx_slider_get_range(fx, index, &slider->range);
    slider->isEnum = ysfx_slider_is_enum(fx, index);
    slider->isPath = ysfx_slider_is_path(fx, index);
    if (slider->isEnum) {
        uint32_t enumSize = ysfx_slider_get_enum_names(fx, index, nullptr, 0);
        std::unique_ptr<const char *[]> enumNames(new const char *[enumSize]);
        ysfx_slider_get_enum_names(fx, index, enumNames.get(), enumSize);
        slider->enumNames.ensureStorageAllocated((int)enumSize);
        for (uint32_t i = 0; i < enumSize; ++i)
            slider->enumNames.add(juce::CharPointer_UTF8(enumNames[i]));
    }
    return slider;
}

//==============================================================================
YsfxInfo::YsfxInfo()
{
    YsfxSliderInfo::Ptr noneSlider = YsfxSliderInfo::none();
    for (uint32_t i = 0; i < ysfx_max_sliders; ++i)
        sliders[i] = noneSlider;
}

YsfxInfo::Ptr YsfxInfo::extractFrom(ysfx_t *fx)
{
    YsfxInfo::Ptr info{new YsfxInfo};
    info->name = juce::CharPointer_UTF8(ysfx_get_name(fx));
    info->path = juce::CharPointer_UTF8(ysfx_get_file_path(fx));
    info->isLoaded = ysfx_is_loaded(fx);
    info->isCompiled = ysfx_is_compiled(fx);
    for (uint32_t i = 0; i < ysfx_max_sliders; ++i)
        info->sliders[i] = YsfxSliderInfo::extractFrom(fx, i);
    return info;
}
