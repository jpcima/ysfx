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
#include <cmath>

void YsfxGraphicsView::configureGfx(int gfxWidth, int gfxHeight, bool gfxWantRetina)
{
    if (m_gfxWidth == gfxWidth && m_gfxHeight == gfxHeight && m_wantRetina == gfxWantRetina)
        return;

    m_gfxWidth = gfxWidth;
    m_gfxHeight = gfxHeight;
    m_wantRetina = gfxWantRetina;

    updateBitmap();
}

void YsfxGraphicsView::paint(juce::Graphics &g)
{
    if (m_bitmapScale == 1)
        g.drawImageAt(m_bitmap, 0, 0);
    else {
        juce::Rectangle<int> dest{0, 0, m_bitmapUnscaledWidth, m_bitmapUnscaledHeight};
        g.drawImage(m_bitmap, dest.toFloat());
    }

    // TODO draw a placeholder thing if the effect does not have @gfx
#if 0
    juce::Rectangle<int> bounds = getLocalBounds();

    g.setColour(juce::Colours::white);
    g.drawRect(bounds);

    juce::Font font;
    font.setHeight(32.0f);

    g.setFont(font);
    g.drawText(TRANS("Graphics not implemented"), bounds, juce::Justification::centred);
#endif
}

void YsfxGraphicsView::resized()
{
    Component::resized();
    updateBitmap();
}

void YsfxGraphicsView::updateBitmap()
{
    int w = m_gfxWidth;
    int h = m_gfxHeight;

    if (w <= 0)
        w = getWidth();
    if (h <= 0)
        h = getHeight();

    m_bitmapUnscaledWidth = w;
    m_bitmapUnscaledHeight = h;

    if (!m_wantRetina)
        m_bitmapScale = 1;
    else {
        m_bitmapScale = 1; // TODO how to get the display scale factor?
        w = (int)std::ceil(w * m_bitmapScale);
        h = (int)std::ceil(h * m_bitmapScale);
    }

    if (m_bitmap.getWidth() != w || m_bitmap.getHeight() != h)
        m_bitmap = juce::Image(juce::Image::ARGB, w, h, true);
}
