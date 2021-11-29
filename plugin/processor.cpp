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

#include "processor.h"
#include "editor.h"
#include "parameter.h"
#include "info.h"
#include "utility/audio_processor_suspender.h"
#include "utility/rt_semaphore.h"
#include "ysfx.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

struct YsfxProcessor::Impl : public juce::AudioProcessorListener {
    YsfxProcessor *m_self = nullptr;
    ysfx_u m_fx;
    ysfx_time_info_t m_timeInfo{};
    int m_sliderParamOffset = 0;
    std::atomic<bool> m_sliderParametersChanged{false};
    YsfxInfo::Ptr m_info{new YsfxInfo};

    //==========================================================================
    void processMidiInput(juce::MidiBuffer &midi);
    void processMidiOutput(juce::MidiBuffer &midi);
    void processSliderChanges();
    void updateTimeInfo();
    void syncParametersToSliders();
    void syncSlidersToParameters();
    void syncParameterToSlider(int index);
    void syncSliderToParameter(int index);

    //==========================================================================
    struct LoadRequest : public std::enable_shared_from_this<LoadRequest> {
        juce::String filePath;
        ysfx_state_u initialState;
        volatile bool completion = false;
        std::mutex completionMutex;
        std::condition_variable completionVariable;
        using Ptr = std::shared_ptr<LoadRequest>;
    };

    LoadRequest::Ptr m_loadRequest;

    //==========================================================================
    class Background {
    public:
        explicit Background(Impl *impl);
        void shutdown();
        void wakeUp();
    private:
        void run();
        Impl *m_impl = nullptr;
        RTSemaphore m_sema;
        std::atomic<bool> m_running{};
        std::thread m_thread;
    };

    std::unique_ptr<Background> m_background;

    //==========================================================================
    void audioProcessorParameterChanged(AudioProcessor *processor, int parameterIndex, float newValue) override;
    void audioProcessorChanged(AudioProcessor *processor, const ChangeDetails &details) override;
};

//==============================================================================
YsfxProcessor::YsfxProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    m_impl(new Impl)
{
    m_impl->m_self = this;

    ysfx_config_u config{ysfx_config_new()};
    ysfx_register_builtin_audio_formats(config.get());

    ysfx_t *fx = ysfx_new(config.get());
    m_impl->m_fx.reset(fx);

    ///
    ysfx_time_info_t &timeInfo = m_impl->m_timeInfo;
    timeInfo.tempo = 120;
    timeInfo.playback_state = ysfx_playback_paused;
    timeInfo.time_position = 0;
    timeInfo.beat_position = 0;
    timeInfo.time_signature[0] = 4;
    timeInfo.time_signature[1] = 4;

    ///
    m_impl->m_sliderParamOffset = getParameters().size();
    for (int i = 0; i < ysfx_max_sliders; ++i)
        addParameter(new YsfxParameter(i));

    ///
    m_impl->m_background.reset(new Impl::Background(m_impl.get()));

    ///
    addListener(m_impl.get());
}

YsfxProcessor::~YsfxProcessor()
{
    removeListener(m_impl.get());

    ///
    m_impl->m_background->shutdown();
}

ysfx_t *YsfxProcessor::getYsfx()
{
    return m_impl->m_fx.get();
}

YsfxParameter *YsfxProcessor::getYsfxParameter(int sliderIndex)
{
    if (sliderIndex < 0 || sliderIndex >= ysfx_max_sliders)
        return nullptr;

    int paramIndex = sliderIndex + m_impl->m_sliderParamOffset;
    return static_cast<YsfxParameter *>(getParameters()[paramIndex]);
}

void YsfxProcessor::loadJsfxFile(const juce::String &filePath, ysfx_state_t *initialState, bool async)
{
    Impl::LoadRequest::Ptr loadRequest{new Impl::LoadRequest};
    loadRequest->filePath = filePath;
    loadRequest->initialState.reset(ysfx_state_dup(initialState));
    std::atomic_store(&m_impl->m_loadRequest, loadRequest);
    m_impl->m_background->wakeUp();
    if (!async) {
        std::unique_lock<std::mutex> lock(loadRequest->completionMutex);
        loadRequest->completionVariable.wait(lock, [&]() { return loadRequest->completion; });
    }
}

YsfxInfo::Ptr YsfxProcessor::getCurrentInfo()
{
    return std::atomic_load(&m_impl->m_info);
}

//==============================================================================
void YsfxProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ScopedLock lock(getCallbackLock());
    ysfx_t *fx = m_impl->m_fx.get();

    ysfx_set_sample_rate(fx, sampleRate);
    ysfx_set_block_size(fx, (uint32_t)samplesPerBlock);

    ysfx_init(fx);
}

void YsfxProcessor::releaseResources()
{
}

void YsfxProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    ysfx_t *fx = m_impl->m_fx.get();

    if (m_impl->m_sliderParametersChanged.exchange(false))
        m_impl->syncParametersToSliders();

    m_impl->updateTimeInfo();
    ysfx_set_time_info(fx, &m_impl->m_timeInfo);

    m_impl->processMidiInput(midiMessages);

    uint32_t numIns = (uint32_t)getTotalNumInputChannels();
    uint32_t numOuts = (uint32_t)getTotalNumOutputChannels();
    uint32_t numFrames = (uint32_t)buffer.getNumSamples();
    ysfx_process_float(fx, buffer.getArrayOfReadPointers(), buffer.getArrayOfWritePointers(), numIns, numOuts, numFrames);

    m_impl->processMidiOutput(midiMessages);
    m_impl->processSliderChanges();
}

void YsfxProcessor::processBlock(juce::AudioBuffer<double> &buffer, juce::MidiBuffer &midiMessages)
{
    ysfx_t *fx = m_impl->m_fx.get();

    if (m_impl->m_sliderParametersChanged.exchange(false))
        m_impl->syncParametersToSliders();

    m_impl->updateTimeInfo();
    ysfx_set_time_info(fx, &m_impl->m_timeInfo);

    m_impl->processMidiInput(midiMessages);

    uint32_t numIns = (uint32_t)getTotalNumInputChannels();
    uint32_t numOuts = (uint32_t)getTotalNumOutputChannels();
    uint32_t numFrames = (uint32_t)buffer.getNumSamples();
    ysfx_process_double(fx, buffer.getArrayOfReadPointers(), buffer.getArrayOfWritePointers(), numIns, numOuts, numFrames);

    m_impl->processMidiOutput(midiMessages);
    m_impl->processSliderChanges();
}

bool YsfxProcessor::supportsDoublePrecisionProcessing() const
{
    return true;
}

//==============================================================================
juce::AudioProcessorEditor *YsfxProcessor::createEditor()
{
    return new YsfxEditor(*this);
}

bool YsfxProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
const juce::String YsfxProcessor::getName() const
{
    return JucePlugin_Name;
}

bool YsfxProcessor::acceptsMidi() const
{
    return true;
}

bool YsfxProcessor::producesMidi() const
{
    return true;
}

bool YsfxProcessor::isMidiEffect() const
{
    return false;
}

double YsfxProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
int YsfxProcessor::getNumPrograms()
{
    return 1;
}

int YsfxProcessor::getCurrentProgram()
{
    return 0;
}

void YsfxProcessor::setCurrentProgram(int index)
{
    (void)index;
}

const juce::String YsfxProcessor::getProgramName(int index)
{
    (void)index;
    return {};
}

void YsfxProcessor::changeProgramName(int index, const juce::String &newName)
{
    (void)index;
    (void)newName;
}

//==============================================================================
void YsfxProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::File path;
    ysfx_state_u state;

    {
        const juce::ScopedLock lock(getCallbackLock());
        ysfx_t *fx = m_impl->m_fx.get();
        path = juce::CharPointer_UTF8(ysfx_get_file_path(fx));
        state.reset(ysfx_save_state(fx));
    }

    juce::ValueTree root("ysfx");
    root.setProperty("version", 1, nullptr);
    root.setProperty("path", path.getFullPathName(), nullptr);

    if (state) {
        juce::ValueTree stateTree("state");

        juce::ValueTree sliderTree("sliders");
        for (uint32_t i = 0; i < state->slider_count; ++i)
            sliderTree.setProperty(juce::String(state->sliders[i].index), state->sliders[i].value, nullptr);
        stateTree.addChild(sliderTree, -1, nullptr);

        stateTree.setProperty("data", juce::Base64::toBase64(state->data, state->data_size), nullptr);

        root.addChild(stateTree, -1, nullptr);
    }

    juce::MemoryOutputStream stream(destData, false);
    root.writeToStream(stream);
}

void YsfxProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    juce::File path;

    juce::MemoryInputStream stream(data, (size_t)sizeInBytes, false);
    juce::ValueTree root = juce::ValueTree::readFromStream(stream);

    if (root.getType().getCharPointer().compare(juce::CharPointer_UTF8("ysfx")) != 0)
        return;
    if ((int)root.getProperty("version") != 1)
        return;

    path = root.getProperty("path").toString();

    juce::ValueTree stateTree = root.getChildWithName("state");
    if (stateTree != juce::ValueTree{}) {
        ysfx_state_t state{};
        juce::Array<ysfx_state_slider_t> sliders;
        juce::MemoryBlock dataBlock;

        {
            juce::ValueTree sliderTree = stateTree.getChildWithName("sliders");
            for (uint32_t i = 0; i < ysfx_max_sliders; ++i) {
                if (const juce::var *v = sliderTree.getPropertyPointer(juce::String(i))) {
                    ysfx_state_slider_t item{};
                    item.index = i;
                    item.value = (double)*v;
                    sliders.add(item);
                }
            }
        }
        {
            juce::MemoryOutputStream base64Result(dataBlock, false);
            juce::Base64::convertFromBase64(base64Result, stateTree.getProperty("data").toString());
        }

        state.sliders = sliders.data();
        state.slider_count = (uint32_t)sliders.size();
        state.data = (uint8_t *)dataBlock.getData();
        state.data_size = dataBlock.getSize();
        loadJsfxFile(path.getFullPathName(), &state, false);
    }
    else {
        loadJsfxFile(path.getFullPathName(), nullptr, false);
    }
}

//==============================================================================
void YsfxProcessor::Impl::processMidiInput(juce::MidiBuffer &midi)
{
    ysfx_t *fx = m_fx.get();

    for (juce::MidiMessageMetadata md : midi) {
        ysfx_midi_event_t event{};
        event.offset = (uint32_t)md.samplePosition;
        event.size = (uint32_t)md.numBytes;
        event.data = md.data;
        ysfx_send_midi(fx, &event);
    }
}

void YsfxProcessor::Impl::processMidiOutput(juce::MidiBuffer &midi)
{
    midi.clear();

    ysfx_midi_event_t event;
    ysfx_t *fx = m_fx.get();
    while (ysfx_receive_midi(fx, &event))
        midi.addEvent(event.data, (int)event.size, (int)event.offset);
}

void YsfxProcessor::Impl::processSliderChanges()
{
    ysfx_t *fx = m_fx.get();

    if (!ysfx_have_slider_changes(fx))
        return;

    for (int i = 0; i < ysfx_max_sliders; ++i) {
        uint32_t type = ysfx_get_slider_change_type(fx, (uint32_t)i);
        if (type & (ysfx_slider_change_display|ysfx_slider_change_automation))
            syncSliderToParameter(i);
        if (type & ysfx_slider_change_visibility)
            ; //TODO: visibility change
    }
}

void YsfxProcessor::Impl::updateTimeInfo()
{
    juce::AudioPlayHead::CurrentPositionInfo cpi;
    if (!m_self->getPlayHead()->getCurrentPosition(cpi))
        return;

    if (cpi.isRecording)
        m_timeInfo.playback_state = ysfx_playback_recording;
    else if (cpi.isPlaying)
        m_timeInfo.playback_state = ysfx_playback_playing;
    else
        m_timeInfo.playback_state = ysfx_playback_paused;

    m_timeInfo.tempo = cpi.bpm;
    m_timeInfo.time_position = cpi.timeInSeconds;
    m_timeInfo.beat_position = cpi.ppqPosition;
    m_timeInfo.time_signature[0] = (uint32_t)cpi.timeSigNumerator;
    m_timeInfo.time_signature[1] = (uint32_t)cpi.timeSigDenominator;
}

void YsfxProcessor::Impl::syncParametersToSliders()
{
    for (int i = 0; i < ysfx_max_sliders; ++i)
        syncParameterToSlider(i);
}

void YsfxProcessor::Impl::syncSlidersToParameters()
{
    for (int i = 0; i < ysfx_max_sliders; ++i)
        syncSliderToParameter(i);
}

void YsfxProcessor::Impl::syncParameterToSlider(int index)
{
    if (index < 0 || index >= ysfx_max_sliders)
        return;

    YsfxParameter *param = m_self->getYsfxParameter(index);
    if (param->existsAsSlider()) {
        ysfx_real actualValue = param->convertToYsfxValue(param->getValue());
        ysfx_slider_set_value(m_fx.get(), (uint32_t)index, actualValue);
    }
}

void YsfxProcessor::Impl::syncSliderToParameter(int index)
{
    if (index < 0 || index >= ysfx_max_sliders)
        return;

    YsfxParameter *param = m_self->getYsfxParameter(index);
    if (param->existsAsSlider()) {
        float normValue = param->convertFromYsfxValue(ysfx_slider_get_value(m_fx.get(), (uint32_t)index));
        param->setValueNotifyingHost(normValue);
    }
}

//==============================================================================
YsfxProcessor::Impl::Background::Background(Impl *impl)
    : m_impl(impl)
{
    m_running.store(true, std::memory_order_relaxed);
    m_thread = std::thread([this]() { run(); });
}

void YsfxProcessor::Impl::Background::shutdown()
{
    m_running.store(false, std::memory_order_relaxed);
    m_sema.post();
    m_thread.join();
}

void YsfxProcessor::Impl::Background::wakeUp()
{
    m_sema.post();
}

void YsfxProcessor::Impl::Background::run()
{
    while (m_sema.wait(), m_running.load(std::memory_order_relaxed)) {
        if (LoadRequest::Ptr loadRequest = std::atomic_exchange(&m_impl->m_loadRequest, LoadRequest::Ptr{})) {
            {
                AudioProcessorSuspender sus{*m_impl->m_self};
                sus.lockCallbacks();
                //
                ysfx_t *fx = m_impl->m_fx.get();
                ysfx_config_t *config = ysfx_get_config(fx);
                ysfx_set_import_root(config, "");
                ysfx_set_data_root(config, "");
                ysfx_guess_file_roots(config, loadRequest->filePath.toRawUTF8());
                //
                uint32_t loadopts = 0;
                uint32_t compileopts = 0;
                ysfx_load_file(fx, loadRequest->filePath.toRawUTF8(), loadopts);
                ysfx_compile(fx, compileopts);
                //
                if (loadRequest->initialState)
                    ysfx_load_state(fx, loadRequest->initialState.get());
                //
                YsfxInfo::Ptr info{YsfxInfo::extractFrom(fx)};
                std::atomic_store(&m_impl->m_info, info);
                //
                for (uint32_t i = 0; i < ysfx_max_sliders; ++i)
                    m_impl->m_self->getYsfxParameter((int)i)->setInfo(*info->sliders[i]);
                //
                m_impl->syncSlidersToParameters();
            }
            std::lock_guard<std::mutex> lock(loadRequest->completionMutex);
            loadRequest->completion = true;
            loadRequest->completionVariable.notify_one();
        }
    }
}

//==============================================================================
void YsfxProcessor::Impl::audioProcessorParameterChanged(AudioProcessor *processor, int parameterIndex, float newValue)
{
    (void)processor;
    (void)parameterIndex;
    (void)newValue;
    m_sliderParametersChanged.store(true);
}

void YsfxProcessor::Impl::audioProcessorChanged(AudioProcessor *processor, const ChangeDetails &details)
{
    (void)processor;
    (void)details;
}

//==============================================================================
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new YsfxProcessor;
}
