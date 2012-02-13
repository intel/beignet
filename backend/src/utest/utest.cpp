/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/**
 * \file utest.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "utest.hpp"
#include "sys/string.hpp"

namespace gbe
{
  std::vector<UTest> *UTest::utestList = NULL;
  void releaseUTestList(void) { if (UTest::utestList) delete UTest::utestList; }

  UTest::UTest(Function fn, const char *name) : fn(fn), name(name) {
    if (utestList == NULL) {
      utestList = new std::vector<UTest>;
      atexit(releaseUTestList);
    }
    utestList->push_back(*this);
  }

  UTest::UTest(void) : fn(NULL), name(NULL) {}

  void UTest::run(const char *name) {
    if (name == NULL) return;
    if (utestList == NULL) return;
    for (size_t i = 0; i < utestList->size(); ++i) {
      const UTest &utest = (*utestList)[i];
      if (utest.name == NULL || utest.fn == NULL) continue;
      if (strequal(utest.name, name)) (utest.fn)();
    }
  }

  void UTest::runAll(void) {
    if (utestList == NULL) return;
    for (size_t i = 0; i < utestList->size(); ++i) {
      const UTest &utest = (*utestList)[i];
      if (utest.fn == NULL) continue;
      (utest.fn)();
    }
  }
} /* namespace gbe */

