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
#include <juce_events/juce_events.h>

namespace better {

class AsyncUpdater : public juce::AsyncUpdater {
public:
    virtual ~AsyncUpdater() override {}

    class Listener {
    public:
        virtual void handleAsyncUpdate(AsyncUpdater *updater) = 0;
    };

    void addListener(Listener *listener);
    void removeListener(Listener *listener);

protected:
    void handleAsyncUpdate() override;

private:
    juce::ListenerList<Listener> m_listeners;
};

} // namespace better
