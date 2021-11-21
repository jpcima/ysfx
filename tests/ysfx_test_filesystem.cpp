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

#include "ysfx_utils.hpp"
#include "ysfx_test_utils.hpp"
#include <catch.hpp>

TEST_CASE("file system utilities", "[filesystem]")
{
    SECTION("case-insensitive path resolution")
    {
        scoped_new_dir root("${root}/fs/");
        scoped_new_dir sub1("${root}/fs/dir1/");
        scoped_new_txt file1("${root}/fs/dir1/file1.txt", "");

        std::string result;

        //----------------------------------------------------------------------

        // exact resolution
        REQUIRE(ysfx::case_resolve(root.m_path.c_str(), "dir1/file1.txt", result) == 1);
        REQUIRE(result == root.m_path + "dir1/file1.txt");
        // inexact resolution (1)
        REQUIRE(ysfx::case_resolve(root.m_path.c_str(), "Dir1/file1.txt", result) == 2);
        REQUIRE(result == root.m_path + "dir1/file1.txt");
        // inexact resolution (2)
        REQUIRE(ysfx::case_resolve(root.m_path.c_str(), "dir1/File1.txt", result) == 2);
        REQUIRE(result == root.m_path + "dir1/file1.txt");
        // inexact resolution (3)
        REQUIRE(ysfx::case_resolve(root.m_path.c_str(), "Dir1/File1.txt", result) == 2);
        REQUIRE(result == root.m_path + "dir1/file1.txt");
        // failed resolution
        REQUIRE(ysfx::case_resolve(root.m_path.c_str(), "dir1/file2.txt", result) == 0);

        //----------------------------------------------------------------------

        // exact resolution
        REQUIRE(ysfx::case_resolve(sub1.m_path.c_str(), "file1.txt", result) == 1);
        REQUIRE(result == sub1.m_path + "file1.txt");
        // inexact resolution
        REQUIRE(ysfx::case_resolve(sub1.m_path.c_str(), "File1.txt", result) == 2);
        REQUIRE(result == sub1.m_path + "file1.txt");
        // failed resolution
        REQUIRE(ysfx::case_resolve(sub1.m_path.c_str(), "file2.txt", result) == 0);

        //----------------------------------------------------------------------

        // exact resolution
        REQUIRE(ysfx::case_resolve(root.m_path.c_str(), "dir1/", result) == 1);
        REQUIRE(result == root.m_path + "dir1/");
        // inexact resolution (1)
        REQUIRE(ysfx::case_resolve(root.m_path.c_str(), "Dir1/", result) == 2);
        REQUIRE(result == root.m_path + "dir1/");
        // failed resolution
        REQUIRE(ysfx::case_resolve(root.m_path.c_str(), "dir2/", result) == 0);
    }
}
