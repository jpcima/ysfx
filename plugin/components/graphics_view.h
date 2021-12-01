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
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <list>

class YsfxGraphicsView : public juce::Component {
public:
    YsfxGraphicsView();
    void setEffect(ysfx_t *fx);

protected:
    void paint(juce::Graphics &g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress &key) override;
    bool keyStateChanged(bool isKeyDown) override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;
    void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

private:
    void configureGfx(int gfxWidth, int gfxHeight, bool gfxWantRetina);
    juce::Image &getBitmap() { return m_bitmap; }
    double getBitmapScale() const { return m_bitmapScale; }
    juce::Point<int> getDisplayOffset() const;

    void updateGfx();
    void updateBitmap();
    void updateYsfxKeyModifiers();
    void updateYsfxMousePosition(const juce::MouseEvent &event);
    void updateYsfxMouseButtons(const juce::MouseEvent &event);
    int showYsfxMenu(const char *desc);

private:
    ysfx_u m_fx;
    std::unique_ptr<juce::Timer> m_gfxTimer;

    int m_gfxWidth = 0;
    int m_gfxHeight = 0;
    bool m_wantRetina = false;
    juce::Image m_bitmap{juce::Image::ARGB, 0, 0, false};
    double m_bitmapScale = 1;
    int m_bitmapUnscaledWidth = 0;
    int m_bitmapUnscaledHeight = 0;

    struct KeyPressed {
        int jcode = 0;
        uint32_t ykey = 0;
        uint32_t ymods = 0;
    };

    std::list<KeyPressed> m_keysPressed;

    uint32_t m_ysfxMouseMods = 0;
    uint32_t m_ysfxMouseButtons = 0;
    int32_t m_ysfxMouseX = 0;
    int32_t m_ysfxMouseY = 0;
    double m_ysfxWheel = 0;
    double m_ysfxHWheel = 0;
};
