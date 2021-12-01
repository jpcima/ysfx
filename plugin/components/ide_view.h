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
#include <memory>

class YsfxIDEView : public juce::Component {
public:
    YsfxIDEView();
    ~YsfxIDEView() override;
    void setEffect(ysfx_t *fx, juce::Time timeStamp);
    void setStatusText(const juce::String &text);
    void focusOnCodeEditor();

    std::function<void(const juce::File &)> onFileSaved;
    std::function<void(const juce::File &)> onReloadRequested;

protected:
    void resized() override;
    void focusOfChildComponentChanged(FocusChangeType cause) override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
