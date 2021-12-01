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
#include <juce_audio_processors/juce_audio_processors.h>

class YsfxParameter final : public juce::RangedAudioParameter {
public:
    explicit YsfxParameter(ysfx_t *fx, int sliderIndex);
    int getSliderIndex() const { return m_sliderIndex; }
    void setEffect(ysfx_t *fx);
    ysfx_t *getEffect() const { return m_fx.get(); }

    bool existsAsSlider() const;
    juce::CharPointer_UTF8 getSliderName() const;
    ysfx_slider_range_t getSliderRange() const;
    bool isEnumSlider() const;
    int getSliderEnumSize() const;
    juce::CharPointer_UTF8 getSliderEnumName(int index) const;

    ysfx_real convertToYsfxValue(float normValue) const;
    float convertFromYsfxValue(ysfx_real actualValue) const;

    const juce::NormalisableRange<float> &getNormalisableRange() const override { return m_range; }
    float getValue() const override;
    void setValue(float newValue) override;
    float getDefaultValue() const override;
    juce::String getText(float normalisedValue, int) const override;
    float getValueForText(const juce::String &text) const override;

private:
    ysfx_u m_fx;
    int m_sliderIndex = 0;
    float m_value = 0.0f;
    const juce::NormalisableRange<float> m_range{0.0f, 1.0f};
};
