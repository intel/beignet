/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#include <stdlib.h>
#include <csignal>
#include <algorithm>
#include <random>
#include <chrono>
#include <iterator>
#include <semaphore.h>
#include <unistd.h>

struct signalMap
{
  const char* signalName;
  int signalNum;
};

using namespace std;
sem_t tag;
vector<UTest> *UTest::utestList = NULL;
vector<int> v;
// Initialize and declare statistics struct
RStatistics UTest::retStatistics;

void releaseUTestList(void) { delete UTest::utestList; }
void runSummaryAtExit(void) {
  // If case crashes, count it as fail, and accumulate finishrun
  if(UTest::retStatistics.finishrun != UTest::utestList->size()) {
    UTest::retStatistics.finishrun++;
   // UTest::retStatistics.failCount++;
  }
  printf("\nsummary:\n----------\n");
  printf("  total: %zu\n",UTest::utestList->size());
  printf("  run: %zu\n",UTest::retStatistics.actualrun);
  printf("  pass: %zu\n",UTest::retStatistics.passCount);
  printf("  fail: %zu\n",UTest::retStatistics.failCount);
  printf("  pass rate: %f\n", (UTest::retStatistics.actualrun)?((float)UTest::retStatistics.passCount/(float)UTest::retStatistics.actualrun):(float)0);
  releaseUTestList();
}

void signalHandler( int signum )
{
  const char* name = "";

  signalMap arr[] = {
    {"SIGILL",  SIGILL},
    {"SIGFPE",  SIGFPE},
    {"SIGABRT", SIGABRT},
    {"SIGBUS",  SIGBUS},
    {"SIGSEGV", SIGSEGV},
    {"SIGHUP",  SIGHUP},
    {"SIGINT",  SIGINT},
    {"SIGQUIT", SIGQUIT},
    {"SIGTERM", SIGTERM},
    {NULL,      -1}
  };

  for(int i=0; arr[i].signalNum != -1 && arr[i].signalName != NULL; i++) {
    if(arr[i].signalNum == signum)

      name = arr[i].signalName;
  }

  printf("    Interrupt signal (%s) received.", name);
  UTest::retStatistics.failCount++;

  exit(signum);
}

void catch_signal(void){
  struct sigaction sa;
  int sigs[] = {
    SIGILL, SIGFPE, SIGABRT, SIGBUS,
    SIGSEGV, SIGHUP, SIGINT, SIGQUIT,
    SIGTERM
  };

  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESETHAND;

  for(unsigned int i = 0; i < sizeof(sigs)/sizeof(sigs[0]); ++i) {
    if (sigaction(sigs[i], &sa, NULL) == -1)
      perror("Could not set signal handler");
  }
}
void *multithread(void * arg)
{
  int SerialNumber;
  //size_t PhtreadNumber = (size_t)arg;

  while(! v.empty()){
    sem_wait(&tag);

    SerialNumber = v.back();
    v.pop_back();

    sem_post(&tag);

    const  UTest &utest = (*UTest::utestList)[SerialNumber];
    if (utest.fn == NULL || utest.haveIssue || utest.isBenchMark) continue;
   // printf("thread%lu  %d, utests.name is %s\n",PhtreadNumber, SerialNumber,utest.name);

    UTest::do_run(utest);
    cl_kernel_destroy(true);
    cl_buffer_destroy();
  }

  return 0;
}


UTest::UTest(Function fn, const char *name, bool isBenchMark, bool haveIssue, bool needDestroyProgram)
       : fn(fn), name(name), isBenchMark(isBenchMark), haveIssue(haveIssue), needDestroyProgram(needDestroyProgram) {

  if (utestList == NULL) {
    utestList = new vector<UTest>;

    catch_signal();
    atexit(runSummaryAtExit);
  }
  utestList->push_back(*this);
}


static bool strequal(const char *s1, const char *s2) {
  if (strcmp(s1, s2) == 0) return true;
  return false;
}

void UTest::do_run(struct UTest utest){
  // Print function name
  printf("%s()", utest.name);
  fflush(stdout);
  retStatistics.actualrun++;
  // Run one case in utestList, print result [SUCCESS] or [FAILED]
  (utest.fn)();
}

void UTest::run(const char *name) {
  if (name == NULL) return;
  if (utestList == NULL) return;

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

void UTest::runMultiThread(const char *number) {
  if (number == NULL) return;
  if (utestList == NULL) return;

  unsigned long i, num;
  sem_init(&tag, 0, 1);

  num = atoi(number);

  unsigned long max_num = sysconf(_SC_NPROCESSORS_ONLN);

  if(num < 1 || num > max_num){
    printf("the value range of multi-thread is [1 - %lu]",max_num);
    return;
  }

  for(i = 0; i < utestList->size(); ++i) v.push_back (i);
  unsigned seed = chrono::system_clock::now ().time_since_epoch ().count ();
  shuffle (v.begin (), v.end (), std::default_random_engine (seed));

  pthread_t pthread_arry[num];

  for(i=0; i<num;i++) pthread_create(&pthread_arry[i], NULL, multithread, (void *)i);
  for(i=0; i<num;i++) pthread_join(pthread_arry[i], NULL);

  sem_destroy(&tag);
}

void UTest::runAll(void) {
  if (utestList == NULL) return;

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

  for (; retStatistics.finishrun < utestList->size(); ++retStatistics.finishrun) {
    const UTest &utest = (*utestList)[retStatistics.finishrun];
    if (utest.fn == NULL || utest.haveIssue || utest.isBenchMark) continue;
    do_run(utest);
    cl_kernel_destroy(utest.needDestroyProgram);
    cl_buffer_destroy();
  }
}

void UTest::runAllBenchMark(void) {
  if (utestList == NULL) return;

  for (; retStatistics.finishrun < utestList->size(); ++retStatistics.finishrun) {
    const UTest &utest = (*utestList)[retStatistics.finishrun];
    if (utest.fn == NULL || utest.haveIssue || !utest.isBenchMark) continue;
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
    if (utest.fn == NULL)
      continue;
    std::cout << utest.name << std::endl;
  }
}
void UTest::listCasesCanRun()
{
  if (utestList == NULL) return;
  for (size_t i = 0; i < utestList->size(); ++i) {
    const UTest &utest = (*utestList)[i];
    if (utest.fn == NULL || utest.haveIssue || utest.isBenchMark)
      continue;
    std::cout << utest.name << std::endl;
  }
}
void UTest::listCasesWithIssue()
{
  if (utestList == NULL) return;
  for (size_t i = 0; i < utestList->size(); ++i) {
    const UTest &utest = (*utestList)[i];
    if (utest.fn == NULL || !utest.haveIssue || utest.isBenchMark)
      continue;
    std::cout << utest.name << std::endl;
  }
}
