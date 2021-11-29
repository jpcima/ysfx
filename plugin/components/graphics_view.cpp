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
#include <ysfx.h>
#include <map>
#include <cmath>
#include <cstdio>

YsfxGraphicsView::YsfxGraphicsView()
{
    setWantsKeyboardFocus(true);
}

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
    juce::Point<int> off = getDisplayOffset();

    if (m_bitmapScale == 1)
        g.drawImageAt(m_bitmap, off.x, off.y);
    else {
        juce::Rectangle<int> dest{off.x, off.y, m_bitmapUnscaledWidth, m_bitmapUnscaledHeight};
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

static uint32_t translateKeyCode(int code)
{
    using Map = std::map<int, uint32_t>;

    static const Map keyCodeMap = []() -> Map
    {
        Map map {
            {juce::KeyPress::F1Key, ysfx_key_f1},
            {juce::KeyPress::F2Key, ysfx_key_f2},
            {juce::KeyPress::F3Key, ysfx_key_f3},
            {juce::KeyPress::F4Key, ysfx_key_f4},
            {juce::KeyPress::F5Key, ysfx_key_f5},
            {juce::KeyPress::F6Key, ysfx_key_f6},
            {juce::KeyPress::F7Key, ysfx_key_f7},
            {juce::KeyPress::F8Key, ysfx_key_f8},
            {juce::KeyPress::F9Key, ysfx_key_f9},
            {juce::KeyPress::F10Key, ysfx_key_f10},
            {juce::KeyPress::F11Key, ysfx_key_f11},
            {juce::KeyPress::F12Key, ysfx_key_f12},
            {juce::KeyPress::leftKey, ysfx_key_left},
            {juce::KeyPress::upKey, ysfx_key_up},
            {juce::KeyPress::rightKey, ysfx_key_right},
            {juce::KeyPress::downKey, ysfx_key_down},
            {juce::KeyPress::pageUpKey, ysfx_key_page_up},
            {juce::KeyPress::pageDownKey, ysfx_key_page_down},
            {juce::KeyPress::homeKey, ysfx_key_home},
            {juce::KeyPress::endKey, ysfx_key_end},
            {juce::KeyPress::insertKey, ysfx_key_insert},
        };
        return map;
    }();

    Map::const_iterator it = keyCodeMap.find(code);
    if (it == keyCodeMap.end())
        return 0;

    return it->second;
}

static uint32_t translateModifiers(juce::ModifierKeys mods)
{
    uint32_t ymods = 0;
    if (mods.isShiftDown())
        ymods |= ysfx_mod_shift;
    if (mods.isCtrlDown())
        ymods |= ysfx_mod_ctrl;
    if (mods.isAltDown())
        ymods |= ysfx_mod_alt;
    if (mods.isCommandDown())
        ymods |= ysfx_mod_super;
    return ymods;
}

static void translateKeyPress(const juce::KeyPress &key, uint32_t &ykey, uint32_t &ymods)
{
    int code = key.getKeyCode();
    juce::juce_wchar character = key.getTextCharacter();
    juce::ModifierKeys mods = key.getModifiers();

    ykey = translateKeyCode(code);
    if (ykey == 0) {
        ykey = (uint32_t)character;
        if (mods.isCtrlDown() && ykey >= 1 && ykey <= 26)
            ykey = ykey - 1 + 'a';
    }

    ymods = translateModifiers(mods);
}

bool YsfxGraphicsView::keyPressed(const juce::KeyPress &key)
{
    for (const KeyPressed &kp : m_keysPressed) {
        if (kp.jcode == key.getKeyCode())
            return true;
    }

    KeyPressed kp;
    kp.jcode = key.getKeyCode();
    translateKeyPress(key, kp.ykey, kp.ymods);

    m_keysPressed.push_back(kp);
    if (OnYsfxKeyPressed)
        OnYsfxKeyPressed(kp.ykey, kp.ymods);

    return true;
}

bool YsfxGraphicsView::keyStateChanged(bool isKeyDown)
{
    if (!isKeyDown) {
        for (auto it = m_keysPressed.begin(); it != m_keysPressed.end(); ) {
            KeyPressed kp = *it;
            if (juce::KeyPress::isKeyCurrentlyDown(kp.jcode))
                ++it;
            else {
                m_keysPressed.erase(it++);
                kp.ymods = translateModifiers(juce::ModifierKeys::getCurrentModifiers());
                if (OnYsfxKeyReleased)
                    OnYsfxKeyReleased(kp.ykey, kp.ymods);
            }
        }
    }

    return true;
}

void YsfxGraphicsView::mouseMove(const juce::MouseEvent &event)
{
    updateYsfxMouseStatus(event);
}

void YsfxGraphicsView::mouseDown(const juce::MouseEvent &event)
{
    updateYsfxMouseStatus(event);
}

void YsfxGraphicsView::mouseDrag(const juce::MouseEvent &event)
{
    updateYsfxMouseStatus(event);
}

void YsfxGraphicsView::mouseUp(const juce::MouseEvent &event)
{
    updateYsfxMouseStatus(event);
}

void YsfxGraphicsView::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    updateYsfxMouseStatus(event);
    YsfxWheel += wheel.deltaY;
    YsfxHWheel += wheel.deltaX;
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

void YsfxGraphicsView::updateYsfxMouseStatus(const juce::MouseEvent &event)
{
    YsfxMouseMods = 0;
    if (event.mods.isShiftDown())
        YsfxMouseMods |= ysfx_mod_shift;
    if (event.mods.isCtrlDown())
        YsfxMouseMods |= ysfx_mod_ctrl;
    if (event.mods.isAltDown())
        YsfxMouseMods |= ysfx_mod_alt;
    if (event.mods.isCommandDown())
        YsfxMouseMods |= ysfx_mod_super;

    YsfxMouseButtons = 0;
    if (event.mods.isLeftButtonDown())
        YsfxMouseButtons |= ysfx_button_left;
    if (event.mods.isMiddleButtonDown())
        YsfxMouseButtons |= ysfx_button_middle;
    if (event.mods.isRightButtonDown())
        YsfxMouseButtons |= ysfx_button_right;

    juce::Point<int> off = getDisplayOffset();
    YsfxMouseX = juce::roundToInt((event.x - off.x) * m_bitmapScale);
    YsfxMouseY = juce::roundToInt((event.y - off.y) * m_bitmapScale);
}

juce::Point<int> YsfxGraphicsView::getDisplayOffset() const
{
    int w = getWidth();
    int h = getHeight();
    int bw = m_bitmapUnscaledWidth;
    int bh = m_bitmapUnscaledHeight;

    juce::Point<int> pt;
    pt.x = (bw < w) ? ((w - bw) / 2) : 0;
    pt.y = (bh < h) ? ((h - bh) / 2) : 0;
    return pt;
}
