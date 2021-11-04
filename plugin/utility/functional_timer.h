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
#include <utility>

class FunctionalTimer : public juce::Timer {
public:
    virtual ~FunctionalTimer() {}
    template <class T> static juce::Timer *create(T fn);
    template <class T> static juce::Timer *create1(T fn);
};

template <class T>
class FunctionalTimerT : public FunctionalTimer {
public:
    explicit FunctionalTimerT(T fn) : m_fn(std::move(fn)) {}
    void timerCallback() override { m_fn(); }
private:
    T m_fn;
};

template <class T>
juce::Timer *FunctionalTimer::create(T fn)
{
    return new FunctionalTimerT<T>(std::move(fn));
}

template <class T>
class FunctionalTimer1T : public FunctionalTimer {
public:
    explicit FunctionalTimer1T(T fn) : m_fn(std::move(fn)) {}
    void timerCallback() override { m_fn(this); }
private:
    T m_fn;
};

template <class T>
juce::Timer *FunctionalTimer::create1(T fn)
{
    return new FunctionalTimer1T<T>(std::move(fn));
}
