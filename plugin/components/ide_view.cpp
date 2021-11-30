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

#include "ide_view.h"
#include "utility/functional_timer.h"
#include <juce_gui_extra/juce_gui_extra.h>

struct YsfxIDEView::Impl {
    YsfxIDEView *m_self = nullptr;
    ysfx_u m_fx;
    std::unique_ptr<juce::CodeDocument> m_document;
    std::unique_ptr<juce::CodeTokeniser> m_tokenizer;
    std::unique_ptr<juce::CodeEditorComponent> m_editor;
    std::unique_ptr<juce::Timer> m_relayoutTimer;

    //==========================================================================
    void setupNewFx();

    //==========================================================================
    void createUI();
    void connectUI();
    void relayoutUI();
    void relayoutUILater();
};

YsfxIDEView::YsfxIDEView()
    : m_impl(new Impl)
{
    m_impl->m_self = this;

    m_impl->m_document.reset(new juce::CodeDocument);

    //TODO code tokenizer

    m_impl->createUI();
    m_impl->connectUI();
    m_impl->relayoutUILater();

    m_impl->setupNewFx();
}

YsfxIDEView::~YsfxIDEView()
{
}

void YsfxIDEView::setEffect(ysfx_t *fx)
{
    if (m_impl->m_fx.get() == fx)
        return;

    m_impl->m_fx.reset(fx);
    if (fx)
        ysfx_add_ref(fx);

    m_impl->setupNewFx();
}

void YsfxIDEView::resized()
{
    m_impl->relayoutUILater();
}

void YsfxIDEView::Impl::setupNewFx()
{
    ysfx_t *fx = m_fx.get();

    m_document->replaceAllContent(juce::String{});
    m_editor->moveCaretToTop(false);

    if (!fx) {
        //
    }
    else {
        juce::File file{juce::CharPointer_UTF8{ysfx_get_file_path(fx)}};
        {
            juce::FileInputStream fileStream(file);
            if (fileStream.openedOk())
                m_document->loadFromStream(fileStream);
        }
    }
}

void YsfxIDEView::Impl::createUI()
{
    m_editor.reset(new juce::CodeEditorComponent(*m_document, m_tokenizer.get()));
    m_self->addAndMakeVisible(*m_editor);
}

void YsfxIDEView::Impl::connectUI()
{
}

void YsfxIDEView::Impl::relayoutUI()
{
    juce::Rectangle<int> bounds = m_self->getLocalBounds();

    //TODO
    m_editor->setBounds(bounds);

    if (m_relayoutTimer)
        m_relayoutTimer->stopTimer();
}

void YsfxIDEView::Impl::relayoutUILater()
{
    if (!m_relayoutTimer)
        m_relayoutTimer.reset(FunctionalTimer::create([this]() { relayoutUI(); }));
    m_relayoutTimer->startTimer(0);
}
