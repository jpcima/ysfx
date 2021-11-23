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

#include "editor.h"
#include "processor.h"
#include "parameter.h"
#include "info.h"
#include "components/parameters_panel.h"
#include "components/graphics_view.h"
#include "utility/functional_timer.h"
#include "ysfx.h"
#include <juce_gui_extra/juce_gui_extra.h>

struct YsfxEditor::Impl {
    YsfxEditor *m_self = nullptr;
    YsfxProcessor *m_proc = nullptr;
    YsfxInfo::Ptr m_info;
    std::unique_ptr<juce::Timer> m_infoTimer;
    std::unique_ptr<juce::Timer> m_gfxTimer;
    std::unique_ptr<juce::FileChooser> m_fileChooser;
    std::unique_ptr<juce::PopupMenu> m_recentFilesPopup;
    bool m_fileChooserActive = false;

    //==========================================================================
    void updateInfo();
    void grabInfoAndUpdate();
    void updateGfx();
    void chooseFileAndLoad();
    void loadFile(const juce::File &file);
    void popupRecentFiles();
    void switchEditor(bool showGfx);
    static juce::File getAppDataDirectory();
    juce::RecentlyOpenedFilesList loadRecentFiles();
    void saveRecentFiles(const juce::RecentlyOpenedFilesList &recent);

    //==========================================================================
    std::unique_ptr<juce::TextButton> m_btnLoadFile;
    std::unique_ptr<juce::TextButton> m_btnRecentFiles;
    std::unique_ptr<juce::TextButton> m_btnSwitchEditor;
    std::unique_ptr<juce::Label> m_lblFilePath;
    std::unique_ptr<juce::Viewport> m_centerViewPort;
    std::unique_ptr<YsfxParametersPanel> m_parametersPanel;
    std::unique_ptr<YsfxGraphicsView> m_graphicsView;

    //==========================================================================
    void createUI();
    void connectUI();
    void relayoutUI();
};

YsfxEditor::YsfxEditor(YsfxProcessor &proc)
    : juce::AudioProcessorEditor(proc),
      m_impl(new Impl)
{
    m_impl->m_self = this;
    m_impl->m_proc = &proc;
    m_impl->m_info = proc.getCurrentInfo();

    setSize(800, 600);
    m_impl->createUI();
    m_impl->connectUI();
    m_impl->relayoutUI();

    m_impl->updateInfo();
}

YsfxEditor::~YsfxEditor()
{
}

void YsfxEditor::Impl::grabInfoAndUpdate()
{
    YsfxInfo::Ptr info = m_proc->getCurrentInfo();
    if (m_info != info) {
        m_info = info;
        updateInfo();
    }
}

void YsfxEditor::Impl::updateGfx()
{
    ysfx_t *fx = m_proc->getYsfx();
    YsfxInfo::Ptr info = m_proc->getCurrentInfo();

    bool gfxWantRetina = ysfx_gfx_wants_retina(fx);
    m_graphicsView->configureGfx(info->gfxWidth, info->gfxHeight, gfxWantRetina);

    juce::Image &bitmap = m_graphicsView->getBitmap();
    bool mustRepaint;

    {
        juce::Image::BitmapData bdata{bitmap, juce::Image::BitmapData::readWrite};

        ysfx_gfx_config_t gc{};
        gc.pixel_width = (uint32_t)bdata.width;
        gc.pixel_height = (uint32_t)bdata.height;
        gc.pixel_stride = (uint32_t)bdata.lineStride;
        gc.pixels = bdata.data;
        gc.scale_factor = m_graphicsView->getBitmapScale();
        ysfx_gfx_setup(fx, &gc);

        mustRepaint = ysfx_gfx_run(fx);
    }

    if (mustRepaint)
        m_graphicsView->repaint();
}

void YsfxEditor::Impl::updateInfo()
{
    if (m_info->path.isNotEmpty())
        m_lblFilePath->setText(m_info->path, juce::dontSendNotification);
    else
        m_lblFilePath->setText(TRANS("No file"), juce::dontSendNotification);

    juce::Array<YsfxParameter *> params;
    params.ensureStorageAllocated(ysfx_max_sliders);
    for (uint32_t i = 0; i < ysfx_max_sliders; ++i) {
        if (m_info->sliders[i]->exists)
            params.add(m_proc->getYsfxParameter((int)i));
    }
    m_parametersPanel->setParametersDisplayed(params);
    m_parametersPanel->setSize(m_centerViewPort->getWidth(), m_centerViewPort->getHeight());
}

void YsfxEditor::Impl::chooseFileAndLoad()
{
    if (m_fileChooserActive)
        return;

    juce::File initialPath;
    juce::File prevFilePath{m_info->path};
    if (prevFilePath != juce::File{})
        initialPath = prevFilePath.getParentDirectory();

    m_fileChooser.reset(new juce::FileChooser(TRANS("Open jsfx..."), initialPath));
    m_fileChooserActive = true;

    m_fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser &chooser) {
            juce::File result = chooser.getResult();
            if (result != juce::File())
                loadFile(result);
            m_fileChooserActive = false;
        });
}

void YsfxEditor::Impl::loadFile(const juce::File &file)
{
    m_proc->loadJsfxFile(file.getFullPathName(), nullptr, true);

    juce::RecentlyOpenedFilesList recent = loadRecentFiles();
    recent.addFile(file);
    saveRecentFiles(recent);
}

void YsfxEditor::Impl::popupRecentFiles()
{
    m_recentFilesPopup.reset(new juce::PopupMenu);

    juce::RecentlyOpenedFilesList recent = loadRecentFiles();
    recent.createPopupMenuItems(*m_recentFilesPopup, 100, false, true);

    if (m_recentFilesPopup->getNumItems() == 0)
        return;

    juce::PopupMenu::Options popupOptions = juce::PopupMenu::Options{}
        .withParentComponent(m_self)
        .withTargetComponent(*m_btnRecentFiles);
    m_recentFilesPopup->showMenuAsync(popupOptions, [this, recent](int index) {
        if (index != 0)
            loadFile(recent.getFile(index - 100));
    });
}

void YsfxEditor::Impl::switchEditor(bool showGfx)
{
    juce::String text = showGfx ? TRANS("Graphics") : TRANS("Sliders");
    m_btnSwitchEditor->setButtonText(text);

    relayoutUI();
}

juce::RecentlyOpenedFilesList YsfxEditor::Impl::loadRecentFiles()
{
    juce::RecentlyOpenedFilesList recent;

    juce::File dir = getAppDataDirectory();
    if (dir == juce::File{})
        return recent;

    juce::File file = dir.getChildFile("PluginRecentFiles.dat");
    juce::FileInputStream stream(file);
    juce::String text = stream.readEntireStreamAsString();
    recent.restoreFromString(text);
    return recent;
}

void YsfxEditor::Impl::saveRecentFiles(const juce::RecentlyOpenedFilesList &recent)
{
    juce::File dir = getAppDataDirectory();
    if (dir == juce::File{})
        return;

    juce::File file = dir.getChildFile("PluginRecentFiles.dat");
    dir.createDirectory();
    juce::FileOutputStream stream(file);
    stream.setPosition(0);
    stream.truncate();
    juce::String text = recent.toString();
    stream.write(text.toRawUTF8(), text.getNumBytesAsUTF8());
}

juce::File YsfxEditor::Impl::getAppDataDirectory()
{
    juce::File dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    if (dir == juce::File{})
        return juce::File{};

    return dir.getChildFile("ysfx");
}

void YsfxEditor::Impl::createUI()
{
    m_btnLoadFile.reset(new juce::TextButton(TRANS("Load")));
    m_self->addAndMakeVisible(*m_btnLoadFile);
    m_btnRecentFiles.reset(new juce::TextButton(TRANS("Recent")));
    m_self->addAndMakeVisible(*m_btnRecentFiles);
    m_btnSwitchEditor.reset(new juce::TextButton(TRANS("Sliders")));
    m_btnSwitchEditor->setClickingTogglesState(true);
    m_self->addAndMakeVisible(*m_btnSwitchEditor);
    m_lblFilePath.reset(new juce::Label);
    m_self->addAndMakeVisible(*m_lblFilePath);
    m_centerViewPort.reset(new juce::Viewport);
    m_self->addAndMakeVisible(*m_centerViewPort);
    m_parametersPanel.reset(new YsfxParametersPanel);
    m_graphicsView.reset(new YsfxGraphicsView);
}

void YsfxEditor::Impl::connectUI()
{
    m_btnLoadFile->onClick = [this]() { chooseFileAndLoad(); };
    m_btnRecentFiles->onClick = [this]() { popupRecentFiles(); };
    m_btnSwitchEditor->onClick = [this]() { switchEditor(m_btnSwitchEditor->getToggleState()); };

    m_infoTimer.reset(FunctionalTimer::create([this]() { grabInfoAndUpdate(); }));
    m_infoTimer->startTimer(100);

    m_gfxTimer.reset(FunctionalTimer::create([this]() { updateGfx(); }));
    m_gfxTimer->startTimerHz(30);
}

void YsfxEditor::Impl::relayoutUI()
{
    juce::Rectangle<int> temp;
    const juce::Rectangle<int> bounds = m_self->getLocalBounds();

    temp = bounds;
    const juce::Rectangle<int> topRow = temp.removeFromTop(50);
    const juce::Rectangle<int> centerArea = temp.withTrimmedLeft(10).withTrimmedRight(10).withTrimmedBottom(10);

    temp = topRow.reduced(10, 10);
    m_btnLoadFile->setBounds(temp.removeFromLeft(100));
    temp.removeFromLeft(10);
    m_btnRecentFiles->setBounds(temp.removeFromLeft(100));
    temp.removeFromLeft(10);
    m_btnSwitchEditor->setBounds(temp.removeFromRight(100));
    temp.removeFromRight(10);
    m_lblFilePath->setBounds(temp);

    m_centerViewPort->setBounds(centerArea);

    juce::Component *viewed;
    if (m_btnSwitchEditor->getToggleState())
        viewed = m_graphicsView.get();
    else
        viewed = m_parametersPanel.get();

    m_centerViewPort->setViewedComponent(viewed, false);
    viewed->setSize(centerArea.getWidth(), centerArea.getHeight());
}
