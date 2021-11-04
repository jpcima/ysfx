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

YsfxParameter::YsfxParameter(int sliderIndex)
    : RangedAudioParameter(
        "slider" + juce::String(sliderIndex + 1),
        "Slider " + juce::String(sliderIndex + 1)),
      m_sliderIndex(sliderIndex)
{
}

bool YsfxParameter::existsAsSlider() const
{
    return m_info.exists;
}

ysfx_real YsfxParameter::convertToYsfxValue(float normValue) const
{
    ysfx_slider_range_t range = m_info.range;
    ysfx_real actualValue = (ysfx_real)normValue * (range.max - range.min) + range.min;
    // NOTE: if enumerated, round the value to nearest,
    //    to make sure imprecision does not land us on the wrong index
    if (m_info.isEnum)
        actualValue = juce::roundToInt(actualValue);
    return actualValue;
}

float YsfxParameter::convertFromYsfxValue(ysfx_real actualValue) const
{
    ysfx_slider_range_t range = m_info.range;
    if (range.min == range.max)
        return 0.0f;
    // NOTE: if enumerated, round value into an index
    if (m_info.isEnum)
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
    ysfx_slider_range_t range = m_info.range;
    ysfx_real actualValue = (ysfx_real)normalisedValue * (range.max - range.min) + range.min;

    if (m_info.isEnum) {
        // NOTE: if enumerated, round the value to nearest,
        //    to make sure imprecision does not land us on the wrong index
        int index = juce::roundToInt(actualValue);
        if (index >= 0 && index < m_info.enumNames.size())
            return m_info.enumNames[index];
    }

    return juce::String(actualValue);
}

float YsfxParameter::getValueForText(const juce::String &text) const
{
    ysfx_slider_range_t range = m_info.range;
    ysfx_real actualValue{};

    bool foundEnum = false;
    if (m_info.isEnum) {
        for (int i = 0; !foundEnum && i < m_info.enumNames.size(); ++i) {
            foundEnum = m_info.enumNames[i] == text;
            if (foundEnum)
                actualValue = i;
        }
    }
    if (!foundEnum)
        actualValue = text.getFloatValue();

    float normValue = (float)((actualValue  - range.min) / (range.max - range.min));
    return normValue;
}
