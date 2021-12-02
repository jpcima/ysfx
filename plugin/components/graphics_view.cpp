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
#include "utility/async_updater.h"
#include "utility/rt_semaphore.h"
#include <list>
#include <map>
#include <queue>
#include <tuple>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <cstdio>

struct YsfxGraphicsView::Impl final : public better::AsyncUpdater::Listener {
    static uint32_t translateKeyCode(int code);
    static uint32_t translateModifiers(juce::ModifierKeys mods);
    static void translateKeyPress(const juce::KeyPress &key, uint32_t &ykey, uint32_t &ymods);

    juce::Point<int> getDisplayOffset() const;

    void tickGfx();
    void updateGfxTarget(int newWidth, int newHeight, int newRetina);
    void updateYsfxKeyModifiers();
    void updateYsfxMousePosition(const juce::MouseEvent &event);
    void updateYsfxMouseButtons(const juce::MouseEvent &event);
    static int showYsfxMenu(void *userdata, const char *desc, int32_t xpos, int32_t ypos);
    static void setYsfxCursor(void *userdata, int32_t cursor);

    YsfxGraphicsView *m_self = nullptr;
    ysfx_u m_fx;
    std::unique_ptr<juce::Timer> m_gfxTimer;

    //--------------------------------------------------------------------------
    struct GfxTarget : public std::enable_shared_from_this<GfxTarget> {
        int m_gfxWidth = 0;
        int m_gfxHeight = 0;
        bool m_wantRetina = false;
        juce::Image m_renderBitmap{juce::Image::ARGB, 0, 0, false};
        double m_bitmapScale = 1;
        int m_bitmapUnscaledWidth = 0;
        int m_bitmapUnscaledHeight = 0;
        using Ptr = std::shared_ptr<GfxTarget>;
    };

    struct GfxInputState : public std::enable_shared_from_this<GfxInputState> {
        uint32_t m_ysfxMouseMods = 0;
        uint32_t m_ysfxMouseButtons = 0;
        int32_t m_ysfxMouseX = 0;
        int32_t m_ysfxMouseY = 0;
        double m_ysfxWheel = 0;
        double m_ysfxHWheel = 0;
        using YsfxKeyEvent = std::tuple<uint32_t, uint32_t, bool>;
        std::queue<YsfxKeyEvent> m_ysfxKeys;
        using Ptr = std::shared_ptr<GfxInputState>;
    };

    GfxTarget::Ptr m_gfxTarget;
    GfxInputState::Ptr m_gfxInputState;

    //--------------------------------------------------------------------------
    struct KeyPressed {
        int jcode = 0;
        uint32_t ykey = 0;
        uint32_t ymods = 0;
    };

    std::list<KeyPressed> m_keysPressed;

    //--------------------------------------------------------------------------
    // Asynchronous popup menu

    static std::unique_ptr<juce::PopupMenu> createPopupMenu(const char **str, int *startid, int menudepth = 0);
    void endPopupMenu(int menuResult);

    std::unique_ptr<juce::PopupMenu> m_popupMenu;

    //--------------------------------------------------------------------------
    // The background thread will trigger these async updates.

    // sends a bitmap the component should repaint itself with
    struct AsyncRepainter : public better::AsyncUpdater {
        // a double-buffer of the render bitmap, copied after a finished rendering
        juce::Image m_bitmap{juce::Image::ARGB, 0, 0, false};
        std::mutex m_mutex;
    };

    // changes the mouse cursor on the component
    struct AsyncMouseCursor : public better::AsyncUpdater {
        std::atomic<juce::MouseCursor::StandardCursorType> m_cursorType;
    };

    // triggers a menu to redisplay
    struct AsyncShowMenu : public better::AsyncUpdater {
        std::string m_menuDesc;
        int m_menuX = 0;
        int m_menuY = 0;
        volatile bool m_completionFlag = false;
        volatile int m_completionValue = 0;
        std::condition_variable m_completionVariable;
        std::mutex m_mutex;
    };

    std::unique_ptr<AsyncRepainter> m_asyncRepainter;
    std::unique_ptr<AsyncMouseCursor> m_asyncMouseCursor;
    std::unique_ptr<AsyncShowMenu> m_asyncShowMenu;

    void handleAsyncUpdate(better::AsyncUpdater *updater) override;

    //--------------------------------------------------------------------------
    // This background thread runs @gfx.
    // This is on a separate thread, because it has elements which can block,
    // which otherwise would require modal loops (eg. `gfx_showmenu`).

    class BackgroundWork {
    public:
        void start();
        void stop();

        struct Message : std::enable_shared_from_this<Message> {
            explicit Message(int type) : m_type{type} {}
            int m_type = 0;
        };

        struct GfxMessage : Message {
            GfxMessage() : Message{'@gfx'} {}
            ysfx_u m_fx;
            GfxTarget::Ptr m_target;
            GfxInputState m_input;
            AsyncRepainter *m_asyncRepainter = nullptr;
            void *m_userData = nullptr;
        };

        void postMessage(std::shared_ptr<Message> message);

    private:
        void run();
        std::shared_ptr<Message> popNextMessage();
        void processGfxMessage(GfxMessage &msg);

    private:
        std::thread m_thread;
        RTSemaphore m_sema;
        volatile bool m_running = false;
        std::queue<std::shared_ptr<Message>> m_messages;
        std::mutex m_messagesMutex;
    };

    BackgroundWork m_work;

    uint32_t m_numWaitedRepaints = 0;
};

YsfxGraphicsView::YsfxGraphicsView()
    : m_impl{new YsfxGraphicsView::Impl}
{
    m_impl->m_self = this;

    m_impl->m_gfxTarget.reset(new Impl::GfxTarget);
    m_impl->m_gfxInputState.reset(new Impl::GfxInputState);

    m_impl->m_asyncRepainter.reset(new Impl::AsyncRepainter);
    m_impl->m_asyncMouseCursor.reset(new Impl::AsyncMouseCursor);
    m_impl->m_asyncShowMenu.reset(new Impl::AsyncShowMenu);

    m_impl->m_asyncRepainter->addListener(m_impl.get());
    m_impl->m_asyncMouseCursor->addListener(m_impl.get());
    m_impl->m_asyncShowMenu->addListener(m_impl.get());

    setWantsKeyboardFocus(true);
}

YsfxGraphicsView::~YsfxGraphicsView()
{
    m_impl->endPopupMenu(0);
    m_impl->m_work.stop();

    m_impl->m_asyncRepainter->removeListener(m_impl.get());
    m_impl->m_asyncMouseCursor->removeListener(m_impl.get());
    m_impl->m_asyncShowMenu->removeListener(m_impl.get());
}

void YsfxGraphicsView::setEffect(ysfx_t *fx)
{
    if (m_impl->m_fx.get() == fx)
        return;

    m_impl->m_fx.reset(fx);
    if (fx)
        ysfx_add_ref(fx);

    m_impl->endPopupMenu(0);
    m_impl->m_work.stop();

    if (!fx || !ysfx_has_section(fx, ysfx_section_gfx)) {
        m_impl->m_gfxTimer.reset();
        repaint();
    }
    else {
        m_impl->m_work.start();
        m_impl->m_gfxTimer.reset(FunctionalTimer::create([this]() { m_impl->tickGfx(); }));
        m_impl->m_gfxTimer->startTimerHz(30);
    }

    m_impl->m_gfxInputState.reset(new Impl::GfxInputState);

    m_impl->m_asyncRepainter->cancelPendingUpdate();
    m_impl->m_asyncMouseCursor->cancelPendingUpdate();
    m_impl->m_asyncShowMenu->cancelPendingUpdate();

    m_impl->m_popupMenu.reset();

    m_impl->m_numWaitedRepaints = 0;

    setMouseCursor(juce::MouseCursor{juce::MouseCursor::NormalCursor});
}

void YsfxGraphicsView::paint(juce::Graphics &g)
{
    ysfx_t *fx = m_impl->m_fx.get();

    if (!(fx && ysfx_has_section(fx, ysfx_section_gfx))) {
        juce::Rectangle<int> bounds = getLocalBounds();

        g.setColour(juce::Colours::white);
        g.drawRect(bounds);

        juce::Font font;
        font.setHeight(32.0f);

        g.setFont(font);
        g.drawText(TRANS("No graphics"), bounds, juce::Justification::centred);
        return;
    }

    ///
    juce::Point<int> off = m_impl->getDisplayOffset();
    Impl::GfxTarget *target = m_impl->m_gfxTarget.get();

    ///
    std::lock_guard<std::mutex> lock{m_impl->m_asyncRepainter->m_mutex};
    juce::Image &image = m_impl->m_asyncRepainter->m_bitmap;

    if (image.getWidth() != target->m_renderBitmap.getWidth() ||
        image.getHeight() != target->m_renderBitmap.getHeight())
    {
        g.fillAll(juce::Colours::black);
    }
    else if (target->m_bitmapScale == 1) {
        g.drawImageAt(image, off.x, off.y);
    }
    else {
        juce::Rectangle<int> dest{off.x, off.y, target->m_bitmapUnscaledWidth, target->m_bitmapUnscaledHeight};
        g.drawImage(image, dest.toFloat());
    }
}

void YsfxGraphicsView::resized()
{
    Component::resized();
    m_impl->updateGfxTarget(-1, -1, -1);
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
    ysfx_t *fx = m_impl->m_fx.get();
    if (fx && ysfx_has_section(fx, ysfx_section_gfx)) {
        Impl::GfxInputState *inputs = m_impl->m_gfxInputState.get();
        inputs->m_ysfxKeys.emplace(kp.ymods, kp.ykey, true);
    }

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
                ysfx_t *fx = m_impl->m_fx.get();
                if (fx && ysfx_has_section(fx, ysfx_section_gfx)) {
                    Impl::GfxInputState *inputs = m_impl->m_gfxInputState.get();
                    inputs->m_ysfxKeys.emplace(kp.ymods, kp.ykey, false);
                }
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

    Impl::GfxInputState *gfxInputState = m_impl->m_gfxInputState.get();
    gfxInputState->m_ysfxMouseButtons = 0;
}

void YsfxGraphicsView::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    m_impl->updateYsfxKeyModifiers();
    m_impl->updateYsfxMousePosition(event);

    Impl::GfxInputState *gfxInputState = m_impl->m_gfxInputState.get();
    gfxInputState->m_ysfxWheel += wheel.deltaY;
    gfxInputState->m_ysfxHWheel += wheel.deltaX;
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
void YsfxGraphicsView::Impl::tickGfx()
{
    // don't overload the background with @gfx requests
    // (remember that @gfx can block)
    if (m_numWaitedRepaints > 1)
        return;

    ysfx_t *fx = m_fx.get();
    jassert(fx);

    ///
    uint32_t gfxDim[2] = {};
    ysfx_get_gfx_dim(fx, gfxDim);

    bool gfxWantRetina = ysfx_gfx_wants_retina(fx);
    updateGfxTarget((int)gfxDim[0], (int)gfxDim[1], gfxWantRetina);

    ///
    std::shared_ptr<BackgroundWork::GfxMessage> msg{new BackgroundWork::GfxMessage};
    msg->m_fx.reset(fx);
    ysfx_add_ref(fx);
    msg->m_target = m_gfxTarget;
    msg->m_input.m_ysfxMouseMods = m_gfxInputState->m_ysfxMouseMods;
    msg->m_input.m_ysfxMouseButtons = m_gfxInputState->m_ysfxMouseButtons;
    msg->m_input.m_ysfxMouseX = m_gfxInputState->m_ysfxMouseX;
    msg->m_input.m_ysfxMouseY = m_gfxInputState->m_ysfxMouseY;
    msg->m_input.m_ysfxWheel = m_gfxInputState->m_ysfxWheel;
    msg->m_input.m_ysfxHWheel = m_gfxInputState->m_ysfxHWheel;
    msg->m_input.m_ysfxKeys = std::move(m_gfxInputState->m_ysfxKeys);
    msg->m_asyncRepainter = m_asyncRepainter.get();
    msg->m_userData = m_self;

    m_gfxInputState->m_ysfxWheel = 0;
    m_gfxInputState->m_ysfxHWheel = 0;

    ///
    m_work.postMessage(msg);
    m_numWaitedRepaints += 1;
}

void YsfxGraphicsView::Impl::updateGfxTarget(int newWidth, int newHeight, int newRetina)
{
    GfxTarget *target = m_gfxTarget.get();

    ///
    newWidth = (newWidth == -1) ? target->m_gfxWidth : newWidth;
    newHeight = (newHeight == -1) ? target->m_gfxHeight : newHeight;
    newRetina = (newRetina == -1) ? target->m_wantRetina : newRetina;

    ///
    bool needsUpdate = false;
    needsUpdate = needsUpdate || (newWidth != target->m_gfxWidth);
    needsUpdate = needsUpdate || (newHeight != target->m_gfxHeight);
    needsUpdate = needsUpdate || (newRetina != target->m_wantRetina);

    ///
    int unscaledWidth = newWidth;
    int unscaledHeight = newHeight;

    if (unscaledWidth <= 0)
        unscaledWidth = m_self->getWidth();
    if (unscaledHeight <= 0)
        unscaledHeight = m_self->getHeight();

    needsUpdate = needsUpdate || (unscaledWidth != target->m_bitmapUnscaledWidth);
    needsUpdate = needsUpdate || (unscaledHeight != target->m_bitmapUnscaledHeight);

    ///
    double bitmapScale = 1;
    int scaledWidth = unscaledWidth;
    int scaledHeight = unscaledHeight;
    if (newRetina) {
        bitmapScale = 1; // TODO how to get the display scale factor?
        scaledWidth = (int)std::ceil(unscaledWidth * bitmapScale);
        scaledHeight = (int)std::ceil(unscaledHeight * bitmapScale);
    }
    needsUpdate = needsUpdate || (target->m_renderBitmap.getWidth() != scaledWidth);
    needsUpdate = needsUpdate || (target->m_renderBitmap.getHeight() != scaledHeight);

    if (needsUpdate) {
        target = new GfxTarget;
        m_gfxTarget.reset(target);
        target->m_gfxWidth = newWidth;
        target->m_gfxHeight = newHeight;
        target->m_wantRetina = (bool)newRetina;
        target->m_renderBitmap = juce::Image(juce::Image::ARGB, scaledWidth, scaledHeight, true);
        target->m_bitmapScale = bitmapScale;
        target->m_bitmapUnscaledWidth = unscaledWidth;
        target->m_bitmapUnscaledHeight = unscaledHeight;
    }
}

void YsfxGraphicsView::Impl::updateYsfxKeyModifiers()
{
    juce::ModifierKeys mods = juce::ModifierKeys::getCurrentModifiers();
    m_gfxInputState->m_ysfxMouseMods = translateModifiers(mods);
}

void YsfxGraphicsView::Impl::updateYsfxMousePosition(const juce::MouseEvent &event)
{
    juce::Point<int> off = getDisplayOffset();
    double bitmapScale = m_gfxTarget->m_bitmapScale;
    m_gfxInputState->m_ysfxMouseX = juce::roundToInt((event.x - off.x) * bitmapScale);
    m_gfxInputState->m_ysfxMouseY = juce::roundToInt((event.y - off.y) * bitmapScale);
}

void YsfxGraphicsView::Impl::updateYsfxMouseButtons(const juce::MouseEvent &event)
{
    uint32_t buttons = 0;
    if (event.mods.isLeftButtonDown())
        buttons |= ysfx_button_left;
    if (event.mods.isMiddleButtonDown())
        buttons |= ysfx_button_middle;
    if (event.mods.isRightButtonDown())
        buttons |= ysfx_button_right;
    m_gfxInputState->m_ysfxMouseButtons = buttons;
}

juce::Point<int> YsfxGraphicsView::Impl::getDisplayOffset() const
{
    int w = m_self->getWidth();
    int h = m_self->getHeight();
    int bw = m_gfxTarget->m_bitmapUnscaledWidth;
    int bh = m_gfxTarget->m_bitmapUnscaledHeight;

    juce::Point<int> pt;
    pt.x = (bw < w) ? ((w - bw) / 2) : 0;
    pt.y = (bh < h) ? ((h - bh) / 2) : 0;
    return pt;
}

int YsfxGraphicsView::Impl::showYsfxMenu(void *userdata, const char *desc, int32_t xpos, int32_t ypos)
{
    YsfxGraphicsView *self = (YsfxGraphicsView *)userdata;
    AsyncShowMenu *updater = self->m_impl->m_asyncShowMenu.get();

    std::unique_lock<std::mutex> lock{updater->m_mutex};
    updater->m_menuDesc.assign(desc);
    updater->m_menuX = xpos;
    updater->m_menuY = ypos;
    updater->m_completionFlag = false;
    updater->m_completionValue = 0;
    updater->triggerAsyncUpdate();

    do updater->m_completionVariable.wait(lock);
    while (!updater->m_completionFlag);

    return updater->m_completionValue;
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

    AsyncMouseCursor *async = self->m_impl->m_asyncMouseCursor.get();
    async->m_cursorType.store(type, std::memory_order_relaxed);
    async->triggerAsyncUpdate();
}

//------------------------------------------------------------------------------
void YsfxGraphicsView::Impl::BackgroundWork::start()
{
    if (m_running)
        return;

    m_running = true;
    m_thread = std::thread([this]() { run(); });
}

void YsfxGraphicsView::Impl::BackgroundWork::stop()
{
    if (!m_running)
        return;

    m_running = false;
    m_sema.post();

    m_thread.join();

    std::lock_guard<std::mutex> lock{m_messagesMutex};
    while (!m_messages.empty())
        m_messages.pop();
}

void YsfxGraphicsView::Impl::BackgroundWork::postMessage(std::shared_ptr<Message> message)
{
    if (!m_running)
        return;

    {
        std::lock_guard<std::mutex> lock{m_messagesMutex};
        m_messages.emplace(message);
    }
    m_sema.post();
}

void YsfxGraphicsView::Impl::BackgroundWork::run()
{
    while (m_sema.wait(), m_running) {
        std::shared_ptr<Message> msg = popNextMessage();
        jassert(msg);

        switch (msg->m_type) {
        case '@gfx':
            processGfxMessage(static_cast<GfxMessage &>(*msg));
            break;
        }
    }
}

std::shared_ptr<YsfxGraphicsView::Impl::BackgroundWork::Message> YsfxGraphicsView::Impl::BackgroundWork::popNextMessage()
{
    std::lock_guard<std::mutex> lock{m_messagesMutex};
    if (m_messages.empty())
        return nullptr;

    std::shared_ptr<Message> msg = m_messages.front();
    m_messages.pop();
    return msg;
}

void YsfxGraphicsView::Impl::BackgroundWork::processGfxMessage(GfxMessage &msg)
{
    ysfx_t *fx = msg.m_fx.get();
    GfxInputState &input = msg.m_input;

    while (!input.m_ysfxKeys.empty()) {
        GfxInputState::YsfxKeyEvent event = input.m_ysfxKeys.front();
        input.m_ysfxKeys.pop();
        ysfx_gfx_add_key(fx, std::get<0>(event), std::get<1>(event), std::get<2>(event));
    }

    ysfx_gfx_update_mouse(fx, input.m_ysfxMouseMods, input.m_ysfxMouseX, input.m_ysfxMouseY, input.m_ysfxMouseButtons, input.m_ysfxWheel, input.m_ysfxHWheel);

    ///
    GfxTarget *target = msg.m_target.get();
    bool mustRepaint;

    {
        juce::Image::BitmapData bdata{target->m_renderBitmap, juce::Image::BitmapData::readWrite};

        ysfx_gfx_config_t gc{};
        gc.user_data = msg.m_userData;
        gc.pixel_width = (uint32_t)bdata.width;
        gc.pixel_height = (uint32_t)bdata.height;
        gc.pixel_stride = (uint32_t)bdata.lineStride;
        gc.pixels = bdata.data;
        gc.scale_factor = target->m_bitmapScale;
        gc.show_menu = &showYsfxMenu;
        gc.set_cursor = &setYsfxCursor;
        ysfx_gfx_setup(fx, &gc);

        mustRepaint = ysfx_gfx_run(fx);
    }

    if (mustRepaint) {
        std::lock_guard<std::mutex> lock{msg.m_asyncRepainter->m_mutex};

        juce::Image &imgsrc = target->m_renderBitmap;
        juce::Image &imgdst = msg.m_asyncRepainter->m_bitmap;

        int w = imgsrc.getWidth();
        int h = imgsrc.getHeight();

        if (w != imgdst.getWidth() || h != imgdst.getHeight())
            imgdst = juce::Image{juce::Image::ARGB, w, h, false};

        juce::Image::BitmapData src{imgsrc, juce::Image::BitmapData::readOnly};
        juce::Image::BitmapData dst{imgdst, juce::Image::BitmapData::writeOnly};

        if (src.lineStride == dst.lineStride) {
            memcpy(dst.data, src.data, (size_t)(h * src.lineStride));
        }
        else {
            for (int row = 0; row < h; ++row)
                memcpy(dst.getLinePointer(row), src.getLinePointer(row), (size_t)(w * src.pixelStride));
        }
    }

    if (mustRepaint)
        msg.m_asyncRepainter->triggerAsyncUpdate();
}

//------------------------------------------------------------------------------
std::unique_ptr<juce::PopupMenu> YsfxGraphicsView::Impl::createPopupMenu(const char **str, int *startid, int menudepth)
{
    if (menudepth >= 8)
        return nullptr;

    std::unique_ptr<juce::PopupMenu> hm{new juce::PopupMenu};
    size_t pos = 0;
    int id = *startid;

    const char *p = *str;
    const char *sep = strchr(p, '|');
    while (sep || *p) {
        size_t len = sep ? (size_t)(sep - p) : strlen(p);
        std::string buf(p, len);
        p += len;
        if (sep)
            sep = strchr(++p, '|');

        const char *q = buf.c_str();
        std::unique_ptr<juce::PopupMenu> subm;
        bool done = false;
        bool enabled = true;
        bool checked = false;
        while (strspn(q, ">#!<")) {
            if (*q == '>' && !subm) {
                subm = createPopupMenu(&p, &id, menudepth + 1);
                sep = strchr(p, '|');
            }
            if (*q == '#')
                enabled = false;
            if (*q == '!')
                checked = true;
            if (*q == '<')
                done = true;
            ++q;
        }
        if (*q) {
            juce::String name{juce::CharPointer_UTF8{q}};
            if (subm)
                hm->addSubMenu(name, std::move(*subm), enabled, nullptr, checked, 0);
            else
                hm->addItem(id++, name, enabled, checked);
        }
        else if (!done)
            hm->addSeparator();
        ++pos;
        if (done)
            break;
    }

    *str = p;
    *startid = id;

    if (!pos)
        return nullptr;

    return hm;
}

void YsfxGraphicsView::Impl::endPopupMenu(int menuResult)
{
    AsyncShowMenu *updater = m_asyncShowMenu.get();

    std::lock_guard<std::mutex> lock{updater->m_mutex};
    updater->m_completionFlag = true;
    updater->m_completionValue = menuResult;
    updater->m_completionVariable.notify_one();
}

//------------------------------------------------------------------------------
void YsfxGraphicsView::Impl::handleAsyncUpdate(better::AsyncUpdater *updater)
{
    if (updater == m_asyncRepainter.get()) {
        m_self->repaint();
        m_numWaitedRepaints -= 1;
    }
    else if (updater == m_asyncMouseCursor.get()) {
        AsyncMouseCursor &cursorUpdater = static_cast<AsyncMouseCursor &>(*updater);
        m_self->setMouseCursor(juce::MouseCursor{cursorUpdater.m_cursorType.load(std::memory_order_relaxed)});
    }
    else if (updater == m_asyncShowMenu.get()) {
        AsyncShowMenu &menuShower = static_cast<AsyncShowMenu &>(*updater);
        std::lock_guard<std::mutex> lock{menuShower.m_mutex};
        int menuId = 1;
        const char *menuDesc = menuShower.m_menuDesc.c_str();
        m_popupMenu = createPopupMenu(&menuDesc, &menuId);

        juce::Point<int> off = getDisplayOffset();
        juce::Point<int> position;
        position.x = juce::roundToInt(menuShower.m_menuX / m_gfxTarget->m_bitmapScale + off.x);
        position.y = juce::roundToInt(menuShower.m_menuY / m_gfxTarget->m_bitmapScale + off.y);
        juce::Point<int> screenPosition = m_self->localPointToGlobal(position);

        m_popupMenu->showMenuAsync(juce::PopupMenu::Options{}
            .withParentComponent(m_self)
            .withTargetScreenArea(juce::Rectangle<int>{screenPosition.x, screenPosition.y, 0, 0}),
            [this](int result) { endPopupMenu(result); });
    }
}
