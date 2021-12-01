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

#include "parameter.h"

YsfxParameter::YsfxParameter(ysfx_t *fx, int sliderIndex)
    : RangedAudioParameter(
        "slider" + juce::String(sliderIndex + 1),
        "Slider " + juce::String(sliderIndex + 1)),
      m_sliderIndex(sliderIndex)
{
    setEffect(fx);
}

void YsfxParameter::setEffect(ysfx_t *fx)
{
    if (m_fx.get() == fx)
        return;

    m_fx.reset(fx);
    if (fx)
        ysfx_add_ref(fx);
}

bool YsfxParameter::existsAsSlider() const
{
    return ysfx_slider_exists(m_fx.get(), (uint32_t)m_sliderIndex);
}

juce::CharPointer_UTF8 YsfxParameter::getSliderName() const
{
    return juce::CharPointer_UTF8{ysfx_slider_get_name(m_fx.get(), (uint32_t)m_sliderIndex)};
}

ysfx_slider_range_t YsfxParameter::getSliderRange() const
{
    ysfx_slider_range_t range{};
    ysfx_slider_get_range(m_fx.get(), (uint32_t)m_sliderIndex, &range);
    return range;
}

bool YsfxParameter::isEnumSlider() const
{
    return ysfx_slider_is_enum(m_fx.get(), (uint32_t)m_sliderIndex);
}

int YsfxParameter::getSliderEnumSize() const
{
    return (int)ysfx_slider_get_enum_names(m_fx.get(), (uint32_t)m_sliderIndex, nullptr, 0);
}

juce::CharPointer_UTF8 YsfxParameter::getSliderEnumName(int index) const
{
    return juce::CharPointer_UTF8{ysfx_slider_get_enum_name(m_fx.get(), (uint32_t)m_sliderIndex, (uint32_t)index)};
}

ysfx_real YsfxParameter::convertToYsfxValue(float normValue) const
{
    ysfx_slider_range_t range = getSliderRange();
    ysfx_real actualValue = (ysfx_real)normValue * (range.max - range.min) + range.min;
    // NOTE: if enumerated, round the value to nearest,
    //    to make sure imprecision does not land us on the wrong index
    if (isEnumSlider())
        actualValue = juce::roundToInt(actualValue);
    return actualValue;
}

float YsfxParameter::convertFromYsfxValue(ysfx_real actualValue) const
{
    ysfx_slider_range_t range = getSliderRange();
    if (range.min == range.max)
        return 0.0f;
    // NOTE: if enumerated, round value into an index
    if (isEnumSlider())
        actualValue = juce::roundToInt(actualValue);
    float normValue = (float)((actualValue  - range.min) / (range.max - range.min));
    return normValue;
}

float YsfxParameter::getValue() const
{
    return m_value;
}

void YsfxParameter::setValue(float newValue)
{
    m_value = newValue;
}

float YsfxParameter::getDefaultValue() const
{
    return 0.0f;
}

juce::String YsfxParameter::getText(float normalisedValue, int) const
{
    ysfx_slider_range_t range = getSliderRange();
    ysfx_real actualValue = (ysfx_real)normalisedValue * (range.max - range.min) + range.min;
    if (isEnumSlider()) {
        int enumSize = getSliderEnumSize();
        // NOTE: if enumerated, round the value to nearest,
        //    to make sure imprecision does not land us on the wrong index
        int index = juce::roundToInt(actualValue);
        if (index >= 0 && index < enumSize)
            return getSliderEnumName(index);
    }

    return juce::String(actualValue);
}

float YsfxParameter::getValueForText(const juce::String &text) const
{
    ysfx_slider_range_t range = getSliderRange();
    ysfx_real actualValue{};

    bool foundEnum = false;
    if (isEnumSlider()) {
        int enumSize = getSliderEnumSize();
        for (int i = 0; !foundEnum && i < enumSize; ++i) {
            foundEnum = text == getSliderEnumName(i);
            if (foundEnum)
                actualValue = i;
        }
    }
    if (!foundEnum)
        actualValue = text.getFloatValue();

    float normValue = (float)((actualValue  - range.min) / (range.max - range.min));
    return normValue;
}
