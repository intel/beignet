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
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>

#define MAX_SUM_LINE 256

using namespace std;
vector<UTest> *UTest::utestList = NULL;
// Initialize and declare statistics struct
RStatistics UTest::retStatistics;

void releaseUTestList(void) { delete UTest::utestList; }
void runAllNoIssueAtExit(void) {
  // If case crashes, count it as fail, and accumulate finishrun
  if(UTest::retStatistics.finishrun != UTest::utestList->size()) {
    UTest::retStatistics.finishrun++;
    UTest::retStatistics.failCount++;
  }
  printf("\nsummary:\n----------\n");
  printf("  total: %zu\n",UTest::utestList->size());
  printf("  run: %zu\n",UTest::retStatistics.finishrun);
  printf("  pass: %zu\n",UTest::retStatistics.passCount);
  printf("  fail: %zu\n",UTest::retStatistics.failCount);
  printf("  pass rate: %f\n",1-(float)UTest::retStatistics.failCount/(float)UTest::utestList->size());
  }

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

void UTest::do_run(struct UTest utest){
  // winsize is a struct in ioctl.h, contains terminal column number
  struct winsize size;
  char spaceList[MAX_SUM_LINE] = {0};

  //Obtain terminal column size
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

  //A string contain MAX_SUM_LINE spaces, to hide the statistic line in stdout
  for (size_t j = 0; j < size.ws_col; j++){
    if ( j >= MAX_SUM_LINE - 1 )  break;
    spaceList[j] = ' ';
    spaceList[j+1] = '\0';
  }
  printf("\r%s\r%s()", spaceList, utest.name);

  // Run one case in utestList
  (utest.fn)();

  // Print dynamic statistics line
  sprintf(spaceList, "\n [run/total: %zu/%zu]\
    pass: %zu; fail: %zu; pass rate: %f\r",
    retStatistics.finishrun+1, utestList->size(),
    retStatistics.passCount,
    retStatistics.failCount,
    1-(float)retStatistics.failCount/(float)utestList->size());

  // If terminal column size lower than length of statistic line, print nothing, If not, print the statistics line
  if (size.ws_col > strlen(spaceList))
    printf("%s", spaceList);
  else
    printf("\n");

  // Refresh console
  fflush(stdout);
}

void UTest::run(const char *name) {
  if (name == NULL) return;
  if (utestList == NULL) return;
  atexit(runAllNoIssueAtExit);

  for (; retStatistics.finishrun < utestList->size(); ++retStatistics.finishrun) {
    const UTest &utest = (*utestList)[retStatistics.finishrun];
    if (utest.name == NULL || utest.fn == NULL ) continue;
    if (strequal(utest.name, name)) {
      do_run(utest);
      cl_kernel_destroy(true);
      cl_buffer_destroy();
    }
  }
}

void UTest::runAll(void) {
  if (utestList == NULL) return;
  atexit(runAllNoIssueAtExit);

  for (; retStatistics.finishrun < utestList->size(); ++retStatistics.finishrun) {
    const UTest &utest = (*utestList)[retStatistics.finishrun];
    if (utest.fn == NULL) continue;
    do_run(utest);
    cl_kernel_destroy(utest.needDestroyProgram);
    cl_buffer_destroy();
  }
}

void UTest::runAllNoIssue(void) {
  if (utestList == NULL) return;
  atexit(runAllNoIssueAtExit);

  for (; retStatistics.finishrun < utestList->size(); ++retStatistics.finishrun) {
    const UTest &utest = (*utestList)[retStatistics.finishrun];
    if (utest.fn == NULL || utest.haveIssue) continue;
    do_run(utest);
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
