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
#include <algorithm>

struct YsfxIDEView::Impl {
    YsfxIDEView *m_self = nullptr;
    ysfx_u m_fx;
    std::unique_ptr<juce::CodeDocument> m_document;
    std::unique_ptr<juce::CodeTokeniser> m_tokenizer;
    std::unique_ptr<juce::CodeEditorComponent> m_editor;
    std::unique_ptr<juce::TextButton> m_btnSave;
    std::unique_ptr<juce::Label> m_lblVariablesHeading;
    std::unique_ptr<juce::Viewport> m_vpVariables;
    std::unique_ptr<juce::Component> m_compVariables;
    std::unique_ptr<juce::Label> m_lblStatus;
    std::unique_ptr<juce::Timer> m_relayoutTimer;

    struct VariableUI {
        ysfx_real *m_var = nullptr;
        juce::String m_name;
        std::unique_ptr<juce::Label> m_lblName;
        std::unique_ptr<juce::Label> m_lblValue;
    };
    juce::Array<VariableUI> m_vars;
    std::unique_ptr<juce::Timer> m_varsUpdateTimer;

    //==========================================================================
    void setupNewFx();
    void saveCurrentFile();

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

void YsfxIDEView::setStatusText(const juce::String &text)
{
    m_impl->m_lblStatus->setText(text, juce::dontSendNotification);
    m_impl->m_lblStatus->setTooltip(text);
}

void YsfxIDEView::resized()
{
    m_impl->relayoutUILater();
}

void YsfxIDEView::Impl::setupNewFx()
{
    ysfx_t *fx = m_fx.get();

    m_vars.clear();
    m_varsUpdateTimer.reset();

    if (!fx) {
        //
        m_document->replaceAllContent(juce::String{});
        m_editor->setReadOnly(true);
    }
    else {
        juce::File file{juce::CharPointer_UTF8{ysfx_get_file_path(fx)}};

        {
            juce::FileInputStream fileStream(file);
            juce::String newContent;
            if (fileStream.openedOk() &&
                (newContent = fileStream.readEntireStreamAsString(),
                 fileStream.getStatus().wasOk()))
            {
                if (newContent != m_document->getAllContent()) {
                    m_document->replaceAllContent(newContent);
                    m_editor->moveCaretToTop(false);
                }
            }
        }

        m_vars.ensureStorageAllocated(64);

        ysfx_enum_vars(fx, +[](const char *name, ysfx_real *var, void *userdata) -> int {
            Impl &impl = *(Impl *)userdata;
            Impl::VariableUI ui;
            ui.m_var = var;
            ui.m_name = juce::CharPointer_UTF8{name};
            ui.m_lblName.reset(new juce::Label(juce::String{}, ui.m_name));
            ui.m_lblName->setTooltip(ui.m_name);
            ui.m_lblName->setMinimumHorizontalScale(1.0f);
            impl.m_compVariables->addAndMakeVisible(*ui.m_lblName);
            ui.m_lblValue.reset(new juce::Label(juce::String{}, "0"));
            impl.m_compVariables->addAndMakeVisible(*ui.m_lblValue);
            impl.m_vars.add(std::move(ui));
            return 1;
        }, this);

        if (!m_vars.isEmpty()) {
            std::sort(
                m_vars.begin(), m_vars.end(),
                [](const VariableUI &a, const VariableUI &b) -> bool {
                    return a.m_name.compareNatural(b.m_name) < 0;
                });

            m_varsUpdateTimer.reset(FunctionalTimer::create([this]() {
                for (int i = 0; i < m_vars.size(); ++i) {
                    VariableUI &ui = m_vars.getReference(i);
                    ui.m_lblValue->setText(juce::String(*ui.m_var), juce::dontSendNotification);
                }
            }));
            m_varsUpdateTimer->startTimer(100);
        }

        m_editor->setReadOnly(false);

        relayoutUILater();
    }
}

void YsfxIDEView::Impl::saveCurrentFile()
{
    ysfx_t *fx = m_fx.get();
    if (!fx)
        return;

    bool success = false;
    juce::File file{juce::CharPointer_UTF8{ysfx_get_file_path(fx)}};

    const juce::String content = m_document->getAllContent();
    {
        juce::FileOutputStream stream{file};
        success = stream.openedOk() &&
            stream.setPosition(0) && stream.truncate().wasOk() &&
            stream.write(content.toRawUTF8(), content.getNumBytesAsUTF8()) &&
            (stream.flush(), stream.getStatus().wasOk());
    }

    if (!success) {
        juce::AlertWindow::showAsync(
            juce::MessageBoxOptions{}
            .withAssociatedComponent(m_self)
            .withIconType(juce::MessageBoxIconType::WarningIcon)
            .withTitle(TRANS("Error"))
            .withButton(TRANS("OK"))
            .withMessage(TRANS("Could not save the JSFX document.")),
            nullptr);
        return;
    }

    if (m_self->onFileSaved)
        m_self->onFileSaved(file);
}

void YsfxIDEView::Impl::createUI()
{
    m_editor.reset(new juce::CodeEditorComponent(*m_document, m_tokenizer.get()));
    m_self->addAndMakeVisible(*m_editor);
    m_btnSave.reset(new juce::TextButton(TRANS("Save")));
    m_btnSave->addShortcut(juce::KeyPress('s', juce::ModifierKeys::ctrlModifier, 0));
    m_self->addAndMakeVisible(*m_btnSave);
    m_lblVariablesHeading.reset(new juce::Label(juce::String{}, TRANS("Variables")));
    m_self->addAndMakeVisible(*m_lblVariablesHeading);
    m_vpVariables.reset(new juce::Viewport);
    m_vpVariables->setScrollBarsShown(true, false);
    m_self->addAndMakeVisible(*m_vpVariables);
    m_compVariables.reset(new juce::Component);
    m_vpVariables->setViewedComponent(m_compVariables.get(), false);
    m_lblStatus.reset(new juce::Label);
    m_lblStatus->setMinimumHorizontalScale(1.0f);
    m_self->addAndMakeVisible(*m_lblStatus);
}

void YsfxIDEView::Impl::connectUI()
{
    m_btnSave->onClick = [this]() { saveCurrentFile(); };
}

void YsfxIDEView::Impl::relayoutUI()
{
    juce::Rectangle<int> temp;
    const juce::Rectangle<int> bounds = m_self->getLocalBounds();

    temp = bounds;
    const juce::Rectangle<int> debugArea = temp.removeFromRight(300);
    const juce::Rectangle<int> topRow = temp.removeFromTop(50);
    const juce::Rectangle<int> statusArea = temp.removeFromBottom(20);
    const juce::Rectangle<int> editArea = temp;

    ///
    temp = topRow.reduced(10, 10);
    m_btnSave->setBounds(temp.removeFromLeft(100));
    temp.removeFromLeft(10);

    ///
    temp = debugArea;
    m_lblVariablesHeading->setBounds(temp.removeFromTop(50).reduced(10, 10));
    m_vpVariables->setBounds(temp.reduced(10, 10));

    const int varRowHeight = 20;
    for (int i = 0; i < m_vars.size(); ++i) {
        VariableUI &var = m_vars.getReference(i);
        juce::Rectangle<int> varRow = juce::Rectangle<int>{}
            .withWidth(m_vpVariables->getWidth())
            .withHeight(varRowHeight)
            .withY(i * varRowHeight);
        juce::Rectangle<int> varTemp = varRow;
        var.m_lblValue->setBounds(varTemp.removeFromRight(100));
        var.m_lblName->setBounds(varTemp);
    }
    m_compVariables->setSize(m_vpVariables->getWidth(), m_vars.size() * varRowHeight);

    m_lblStatus->setBounds(statusArea);

    m_editor->setBounds(editArea);

    if (m_relayoutTimer)
        m_relayoutTimer->stopTimer();
}

void YsfxIDEView::Impl::relayoutUILater()
{
    if (!m_relayoutTimer)
        m_relayoutTimer.reset(FunctionalTimer::create([this]() { relayoutUI(); }));
    m_relayoutTimer->startTimer(0);
}
