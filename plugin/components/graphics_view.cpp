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

#include "graphics_view.h"

void YsfxGraphicsView::paint(juce::Graphics &g)
{
    juce::Rectangle<int> bounds = getLocalBounds();

    g.setColour(juce::Colours::white);
    g.drawRect(bounds);

    juce::Font font;
    font.setHeight(32.0f);

    g.setFont(font);
    g.drawText(TRANS("Graphics not implemented"), bounds, juce::Justification::centred);
}
