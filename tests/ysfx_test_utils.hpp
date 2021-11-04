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
#include <string>

extern std::string tests_root_path;

//------------------------------------------------------------------------------
struct scoped_new_dir {
    explicit scoped_new_dir(const std::string &path);
    ~scoped_new_dir();
    std::string m_path;
};

//------------------------------------------------------------------------------
struct scoped_new_txt {
    scoped_new_txt(const std::string &path, const char *text, size_t size = ~(size_t)0);
    ~scoped_new_txt();
    std::string m_path;
};
