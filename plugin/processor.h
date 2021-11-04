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
#include "info.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
class YsfxParameter;
using ysfx_state_t = struct ysfx_state_s;

class YsfxProcessor : public juce::AudioProcessor {
public:
    YsfxProcessor();
    ~YsfxProcessor() override;

    YsfxParameter *getYsfxParameter(int sliderIndex);
    void loadJsfxFile(const juce::String &filePath, ysfx_state_t *initialState, bool async);
    YsfxInfo::Ptr getCurrentInfo();

    //==========================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
    void processBlock(juce::AudioBuffer<double> &buffer, juce::MidiBuffer &midiMessages) override;
    bool supportsDoublePrecisionProcessing() const override;

    //==========================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==========================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==========================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==========================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(YsfxProcessor)
};
