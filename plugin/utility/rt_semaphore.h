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
#if defined(__APPLE__)
#include <mach/mach.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <semaphore.h>
#endif
#include <cstdint>
#include <system_error>

class RTSemaphore {
public:
    explicit RTSemaphore(unsigned value = 0);
    explicit RTSemaphore(std::error_code& ec, unsigned value = 0) noexcept;
    ~RTSemaphore() noexcept;

    RTSemaphore(const RTSemaphore&) = delete;
    RTSemaphore& operator=(const RTSemaphore&) = delete;

    explicit operator bool() const noexcept { return good_; }

    void post();
    void wait();
    bool try_wait();
    bool timed_wait(uint32_t milliseconds);

    void post(std::error_code& ec) noexcept;
    void wait(std::error_code& ec) noexcept;
    bool try_wait(std::error_code& ec) noexcept;
    bool timed_wait(uint32_t milliseconds, std::error_code& ec) noexcept;

private:
    void init(std::error_code& ec, unsigned value);
    void destroy(std::error_code& ec);

private:
#if defined(__APPLE__)
    semaphore_t sem_ {};
    static const std::error_category& mach_category();
#elif defined(_WIN32)
    HANDLE sem_ {};
#else
    sem_t sem_ {};
#endif
    bool good_ {};
};
