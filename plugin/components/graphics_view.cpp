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
#include "utility/functional_timer.h"
#include <list>
#include <map>
#include <queue>
#include <tuple>
#include <cmath>
#include <cstdio>

struct YsfxGraphicsView::Impl {
    static uint32_t translateKeyCode(int code);
    static uint32_t translateModifiers(juce::ModifierKeys mods);
    static void translateKeyPress(const juce::KeyPress &key, uint32_t &ykey, uint32_t &ymods);

    void configureGfx(int gfxWidth, int gfxHeight, bool gfxWantRetina);
    juce::Image &getBitmap() { return m_bitmap; }
    double getBitmapScale() const { return m_bitmapScale; }
    juce::Point<int> getDisplayOffset() const;

    void updateGfx();
    void updateBitmap();
    void updateYsfxKeyModifiers();
    void updateYsfxMousePosition(const juce::MouseEvent &event);
    void updateYsfxMouseButtons(const juce::MouseEvent &event);
    static int showYsfxMenu(void *userdata, const char *desc, int32_t xpos, int32_t ypos);
    static void setYsfxCursor(void *userdata, int32_t cursor);

    YsfxGraphicsView *m_self = nullptr;
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
    using YsfxKeyEvent = std::tuple<uint32_t, uint32_t, bool>;
    std::queue<YsfxKeyEvent> m_ysfxKeys;
};

YsfxGraphicsView::YsfxGraphicsView()
    : m_impl{new YsfxGraphicsView::Impl}
{
    m_impl->m_self = this;

    setWantsKeyboardFocus(true);
}

YsfxGraphicsView::~YsfxGraphicsView()
{
}

void YsfxGraphicsView::setEffect(ysfx_t *fx)
{
    if (m_impl->m_fx.get() == fx)
        return;

    m_impl->m_fx.reset(fx);
    if (fx)
        ysfx_add_ref(fx);

    if (!fx || !ysfx_has_section(fx, ysfx_section_gfx)) {
        m_impl->m_gfxTimer.reset();
        repaint();
    }
    else {
        m_impl->m_gfxTimer.reset(FunctionalTimer::create([this]() { m_impl->updateGfx(); }));
        m_impl->m_gfxTimer->startTimerHz(30);
    }

    while (!m_impl->m_ysfxKeys.empty())
        m_impl->m_ysfxKeys.pop();
}

void YsfxGraphicsView::paint(juce::Graphics &g)
{
    ysfx_t *fx = m_impl->m_fx.get();

    if (fx && ysfx_has_section(fx, ysfx_section_gfx)) {
        juce::Point<int> off = m_impl->getDisplayOffset();

        if (m_impl->m_bitmapScale == 1)
            g.drawImageAt(m_impl->m_bitmap, off.x, off.y);
        else {
            juce::Rectangle<int> dest{off.x, off.y, m_impl->m_bitmapUnscaledWidth, m_impl->m_bitmapUnscaledHeight};
            g.drawImage(m_impl->m_bitmap, dest.toFloat());
        }
    }
    else {
        juce::Rectangle<int> bounds = getLocalBounds();

        g.setColour(juce::Colours::white);
        g.drawRect(bounds);

        juce::Font font;
        font.setHeight(32.0f);

        g.setFont(font);
        g.drawText(TRANS("No graphics"), bounds, juce::Justification::centred);
    }
}

void YsfxGraphicsView::resized()
{
    Component::resized();
    m_impl->updateBitmap();
}

bool YsfxGraphicsView::keyPressed(const juce::KeyPress &key)
{
    m_impl->updateYsfxKeyModifiers();

    for (const Impl::KeyPressed &kp : m_impl->m_keysPressed) {
        if (kp.jcode == key.getKeyCode())
            return true;
    }

    Impl::KeyPressed kp;
    kp.jcode = key.getKeyCode();
    Impl::translateKeyPress(key, kp.ykey, kp.ymods);

    m_impl->m_keysPressed.push_back(kp);
    if (m_impl->m_gfxTimer && m_impl->m_gfxTimer->isTimerRunning())
        m_impl->m_ysfxKeys.emplace(kp.ymods, kp.ykey, true);

    return true;
}

bool YsfxGraphicsView::keyStateChanged(bool isKeyDown)
{
    m_impl->updateYsfxKeyModifiers();

    if (!isKeyDown) {
        for (auto it = m_impl->m_keysPressed.begin(); it != m_impl->m_keysPressed.end(); ) {
            Impl::KeyPressed kp = *it;
            if (juce::KeyPress::isKeyCurrentlyDown(kp.jcode))
                ++it;
            else {
                m_impl->m_keysPressed.erase(it++);
                kp.ymods = Impl::translateModifiers(juce::ModifierKeys::getCurrentModifiers());
                if (m_impl->m_gfxTimer && m_impl->m_gfxTimer->isTimerRunning())
                    m_impl->m_ysfxKeys.emplace(kp.ymods, kp.ykey, false);
            }
        }
    }

    return true;
}

void YsfxGraphicsView::mouseMove(const juce::MouseEvent &event)
{
    m_impl->updateYsfxKeyModifiers();
    m_impl->updateYsfxMousePosition(event);
}

void YsfxGraphicsView::mouseDown(const juce::MouseEvent &event)
{
    m_impl->updateYsfxKeyModifiers();
    m_impl->updateYsfxMousePosition(event);
    m_impl->updateYsfxMouseButtons(event);
}

void YsfxGraphicsView::mouseDrag(const juce::MouseEvent &event)
{
    m_impl->updateYsfxKeyModifiers();
    m_impl->updateYsfxMousePosition(event);
}

void YsfxGraphicsView::mouseUp(const juce::MouseEvent &event)
{
    m_impl->updateYsfxKeyModifiers();
    m_impl->updateYsfxMousePosition(event);
    m_impl->m_ysfxMouseButtons = 0;
}

void YsfxGraphicsView::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    m_impl->updateYsfxKeyModifiers();
    m_impl->updateYsfxMousePosition(event);
    m_impl->m_ysfxWheel += wheel.deltaY;
    m_impl->m_ysfxHWheel += wheel.deltaX;
}

//------------------------------------------------------------------------------
uint32_t YsfxGraphicsView::Impl::translateKeyCode(int code)
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

uint32_t YsfxGraphicsView::Impl::translateModifiers(juce::ModifierKeys mods)
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

void YsfxGraphicsView::Impl::translateKeyPress(const juce::KeyPress &key, uint32_t &ykey, uint32_t &ymods)
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

//------------------------------------------------------------------------------
void YsfxGraphicsView::Impl::configureGfx(int gfxWidth, int gfxHeight, bool gfxWantRetina)
{
    if (m_gfxWidth == gfxWidth && m_gfxHeight == gfxHeight && m_wantRetina == gfxWantRetina)
        return;

    m_gfxWidth = gfxWidth;
    m_gfxHeight = gfxHeight;
    m_wantRetina = gfxWantRetina;

    updateBitmap();
}

void YsfxGraphicsView::Impl::updateGfx()
{
    ysfx_t *fx = m_fx.get();
    jassert(fx);

    ///
    while (!m_ysfxKeys.empty()) {
        YsfxKeyEvent event = m_ysfxKeys.front();
        m_ysfxKeys.pop();
        ysfx_gfx_add_key(fx, std::get<0>(event), std::get<1>(event), std::get<2>(event));
    }

    ///
    ysfx_gfx_update_mouse(fx, m_ysfxMouseMods, m_ysfxMouseX, m_ysfxMouseY, m_ysfxMouseButtons, m_ysfxWheel, m_ysfxHWheel);
    m_ysfxWheel = 0;
    m_ysfxHWheel = 0;

    ///
    uint32_t gfxDim[2] = {};
    ysfx_get_gfx_dim(fx, gfxDim);

    bool gfxWantRetina = ysfx_gfx_wants_retina(fx);
    configureGfx((int)gfxDim[0], (int)gfxDim[1], gfxWantRetina);

    juce::Image &bitmap = getBitmap();
    bool mustRepaint;

    {
        juce::Image::BitmapData bdata{bitmap, juce::Image::BitmapData::readWrite};

        ysfx_gfx_config_t gc{};
        gc.user_data = m_self;
        gc.pixel_width = (uint32_t)bdata.width;
        gc.pixel_height = (uint32_t)bdata.height;
        gc.pixel_stride = (uint32_t)bdata.lineStride;
        gc.pixels = bdata.data;
        gc.scale_factor = getBitmapScale();
        gc.show_menu = &showYsfxMenu;
        gc.set_cursor = &setYsfxCursor;
        ysfx_gfx_setup(fx, &gc);

        mustRepaint = ysfx_gfx_run(fx);
    }

    if (mustRepaint)
        m_self->repaint();
}

void YsfxGraphicsView::Impl::updateBitmap()
{
    int w = m_gfxWidth;
    int h = m_gfxHeight;

    if (w <= 0)
        w = m_self->getWidth();
    if (h <= 0)
        h = m_self->getHeight();

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

void YsfxGraphicsView::Impl::updateYsfxKeyModifiers()
{
    juce::ModifierKeys mods = juce::ModifierKeys::getCurrentModifiers();
    m_ysfxMouseMods = translateModifiers(mods);
}

void YsfxGraphicsView::Impl::updateYsfxMousePosition(const juce::MouseEvent &event)
{
    juce::Point<int> off = getDisplayOffset();
    m_ysfxMouseX = juce::roundToInt((event.x - off.x) * m_bitmapScale);
    m_ysfxMouseY = juce::roundToInt((event.y - off.y) * m_bitmapScale);
}

void YsfxGraphicsView::Impl::updateYsfxMouseButtons(const juce::MouseEvent &event)
{
    m_ysfxMouseButtons = 0;
    if (event.mods.isLeftButtonDown())
        m_ysfxMouseButtons |= ysfx_button_left;
    if (event.mods.isMiddleButtonDown())
        m_ysfxMouseButtons |= ysfx_button_middle;
    if (event.mods.isRightButtonDown())
        m_ysfxMouseButtons |= ysfx_button_right;
}

juce::Point<int> YsfxGraphicsView::Impl::getDisplayOffset() const
{
    int w = m_self->getWidth();
    int h = m_self->getHeight();
    int bw = m_bitmapUnscaledWidth;
    int bh = m_bitmapUnscaledHeight;

    juce::Point<int> pt;
    pt.x = (bw < w) ? ((w - bw) / 2) : 0;
    pt.y = (bh < h) ? ((h - bh) / 2) : 0;
    return pt;
}

int YsfxGraphicsView::Impl::showYsfxMenu(void *userdata, const char *desc, int32_t xpos, int32_t ypos)
{
    YsfxGraphicsView *self = (YsfxGraphicsView *)userdata;
    //TODO implement me: popup menu
    (void)self;
    (void)desc;
    (void)xpos;
    (void)ypos;
    return 0;
}

void YsfxGraphicsView::Impl::setYsfxCursor(void *userdata, int32_t cursor)
{
    YsfxGraphicsView *self = (YsfxGraphicsView *)userdata;

    enum {
        ocr_normal = 32512,
        ocr_ibeam = 32513,
        ocr_wait = 32514,
        ocr_cross = 32515,
        ocr_up = 32516,
        ocr_size = 32640,
        ocr_icon = 32641,
        ocr_sizenwse = 32642,
        ocr_sizenesw = 32643,
        ocr_sizewe = 32644,
        ocr_sizens = 32645,
        ocr_sizeall = 32646,
        ocr_icocur = 32647,
        ocr_no = 32648,
        ocr_hand = 32649,
        ocr_appstarting = 32650,
    };

    using CursorType = juce::MouseCursor::StandardCursorType;
    CursorType type;

    switch (cursor) {
    default:
    case ocr_normal:
    case ocr_icon:
        type = CursorType::NormalCursor;
        break;
    case ocr_ibeam:
        type = CursorType::IBeamCursor;
        break;
    case ocr_wait:
        type = CursorType::WaitCursor;
        break;
    case ocr_cross:
        type = CursorType::CrosshairCursor;
        break;
    case ocr_size:
    case ocr_sizeall:
        type = CursorType::UpDownLeftRightResizeCursor;
        break;
    case ocr_sizenwse:
        type = CursorType::TopLeftCornerResizeCursor;
        break;
    case ocr_sizenesw:
        type = CursorType::TopRightCornerResizeCursor;
        break;
    case ocr_sizewe:
        type = CursorType::LeftRightResizeCursor;
        break;
    case ocr_sizens:
        type = CursorType::UpDownResizeCursor;
        break;
    case ocr_hand:
        type = CursorType::PointingHandCursor;
        break;
    }

    self->setMouseCursor(juce::MouseCursor{type});
}
