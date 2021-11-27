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
#include <juce_audio_processors/juce_audio_processors.h>
class YsfxParameter;
class YsfxParameterDisplayComponent;

class YsfxParametersPanel : public juce::Component {
public:
    explicit YsfxParametersPanel();
    ~YsfxParametersPanel() override;

    void setParametersDisplayed(const juce::Array<YsfxParameter *> &parameters);

    int getRecommendedHeight(int heightAtLeast = 125) const;

protected:
    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    juce::OwnedArray<YsfxParameterDisplayComponent> paramComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(YsfxParametersPanel)
};
