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
 * \file utest.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "utest.hpp"
#include "utest_helper.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <cstring>

using namespace std;
vector<UTest> *UTest::utestList = NULL;
void releaseUTestList(void) { delete UTest::utestList; }

UTest::UTest(Function fn, const char *name, bool haveIssue, bool needDestroyProgram)
       : fn(fn), name(name), haveIssue(haveIssue), needDestroyProgram(needDestroyProgram) {
  if (utestList == NULL) {
    utestList = new vector<UTest>;
    atexit(releaseUTestList);
  }
  utestList->push_back(*this);
}

static bool strequal(const char *s1, const char *s2) {
  if (strcmp(s1, s2) == 0) return true;
  return false;
}

void UTest::run(const char *name) {
  if (name == NULL) return;
  if (utestList == NULL) return;
  for (size_t i = 0; i < utestList->size(); ++i) {
    const UTest &utest = (*utestList)[i];
    if (utest.name == NULL || utest.fn == NULL ) continue;
    if (strequal(utest.name, name)) {
      std::cout << utest.name << ":" << std::endl;
      (utest.fn)();
      std::cout << std::endl;
      cl_kernel_destroy(true);
      cl_buffer_destroy();
    }
  }
}

void UTest::runAll(void) {
  if (utestList == NULL) return;
  for (size_t i = 0; i < utestList->size(); ++i) {
    const UTest &utest = (*utestList)[i];
    if (utest.fn == NULL) continue;
    std::cout << utest.name << ":" << std::endl;
    (utest.fn)();
    std::cout << std::endl;
    cl_kernel_destroy(utest.needDestroyProgram);
    cl_buffer_destroy();
  }
}

void UTest::runAllNoIssue(void) {
  if (utestList == NULL) return;
  for (size_t i = 0; i < utestList->size(); ++i) {
    const UTest &utest = (*utestList)[i];
    if (utest.fn == NULL || utest.haveIssue) continue;
    std::cout << utest.name << ":" << std::endl;
    (utest.fn)();
    std::cout << std::endl;
    cl_kernel_destroy(utest.needDestroyProgram);
    cl_buffer_destroy();
  }
}

void UTest::listAllCases()
{
  if (utestList == NULL) return;
    for (size_t i = 0; i < utestList->size(); ++i) {
      const UTest &utest = (*utestList)[i];
      if (utest.fn == NULL) continue;
    std::cout << utest.name << std::endl;
 }
}
