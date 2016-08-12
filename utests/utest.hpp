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
 * \file utest.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Provides all unit test capabilites. It is rather rudimentary but it should
 * do the job
 */
#ifndef __UTEST_UTEST_HPP__
#define __UTEST_UTEST_HPP__

#include "utest_exception.hpp"
#include <vector>
#include <iostream>
#include <iomanip>

/*! struct for statistics */
struct RStatistics
{
  size_t passCount;
  size_t failCount;
  size_t finishrun;
  size_t actualrun;
};

/*! Quick and dirty unit test system with registration */
struct UTest
{
  /*! A unit test function to run */
  typedef void (*Function) (void);
  /*! Empty test */
  UTest(void);
  /*! Build a new unit test and append it to the unit test list */
  UTest(Function fn, const char *name, bool isBenchMark = false, bool haveIssue = false, bool needDestroyProgram = true);
  /*! Function to execute */
  Function fn;
  /*! Name of the test */
  const char *name;
  /*! numbers of the jobs */
  const char *number;
  /*! whether it is a bench mark. */
  bool isBenchMark;
  /*! Indicate whether current test cases has issue to be fixes */
  bool haveIssue;
  /*! Indicate whether destroy kernels/program. */
  bool needDestroyProgram;
  /*! The tests that are registered */
  static std::vector<UTest> *utestList;
  /*! Run the test with the given name */
  static void run(const char *name);
  /*! Run the test with the given name */
  static void runMultiThread(const char *number);
  /*! Run all the tests without known issue*/
  static void runAllNoIssue(void);
  /*! Run all the benchmark. */
  static void runAllBenchMark(void);
  /*! Run all the tests */
  static void runAll(void);
  /*! List all test cases */
  static void listAllCases(void);
  /*! List test cases that can run*/
  static void listCasesCanRun(void);
  /*! List test cases with issue*/
  static void listCasesWithIssue(void);
  /*! Statistics struct */
  static RStatistics retStatistics;
  /*! Do run a test case actually */
  static void do_run(struct UTest utest);
};

/*! Register a new unit test */
#define UTEST_REGISTER(FN) static const UTest __##FN##__(FN, #FN);

#define MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(FN, KEEP_PROGRAM) \
  static void __ANON__##FN##__(void) { UTEST_EXPECT_SUCCESS(FN()); } \
  static const UTest __##FN##__(__ANON__##FN##__, #FN, false, false, !(KEEP_PROGRAM));


/*! Turn a function into a unit test */
#define MAKE_UTEST_FROM_FUNCTION(FN) \
  static void __ANON__##FN##__(void) { UTEST_EXPECT_SUCCESS(FN()); } \
  static const UTest __##FN##__(__ANON__##FN##__, #FN);

/*! Register a test case which has issue to be fixed */
#define MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(FN) \
  static void __ANON__##FN##__(void) { UTEST_EXPECT_SUCCESS(FN()); } \
  static const UTest __##FN##__(__ANON__##FN##__, #FN, false ,true);

/*! Turn a function into a unit performance test */
#define MAKE_BENCHMARK_FROM_FUNCTION_KEEP_PROGRAM(FN, KEEP_PROGRAM, ...) \
  static void __ANON__##FN##__(void) { BENCHMARK(FN(), __VA_ARGS__); } \
  static const UTest __##FN##__(__ANON__##FN##__, #FN, true, false, !(KEEP_PROGRAM));

#define MAKE_BENCHMARK_FROM_FUNCTION(FN, ...) \
  static void __ANON__##FN##__(void) { BENCHMARK(FN(), __VA_ARGS__); } \
  static const UTest __##FN##__(__ANON__##FN##__, #FN, true);


/*! No assert is expected */
#define UTEST_EXPECT_SUCCESS(EXPR) \
 do { \
    try { \
      EXPR; \
      std::cout << "    [SUCCESS]" << std::endl; \
      UTest::retStatistics.passCount += 1; \
    } \
    catch (Exception e) { \
      std::cout << "    [FAILED]" << std::endl; \
      std::cout << "    " << e.what() << std::endl; \
      UTest::retStatistics.failCount++; \
    } \
  } while (0)

#define UTEST_EXPECT_FAILED(EXPR) \
 do { \
    try { \
      EXPR; \
      std::cout << "    [FAILED]" << std::endl; \
      retStatistics.failCount++; \
    } \
    catch (gbe::Exception e) { \
      std::cout << "    [SUCCESS]" << std::endl; \
      retStatistics.passCount++; \
    } \
  } while (0)

#define BENCHMARK(EXPR, ...) \
 do { \
    double ret = 0;\
    try { \
      ret = EXPR; \
      std::cout << "    [Result: " << std::fixed<< std::setprecision(3) << ret << " " << __VA_ARGS__ << "]    [SUCCESS]" << std::endl; \
      UTest::retStatistics.passCount += 1; \
    } \
    catch (Exception e) { \
      std::cout << "  " << #EXPR << "    [FAILED]" << std::endl; \
      std::cout << "    " << e.what() << std::endl; \
      UTest::retStatistics.failCount++; \
    } \
  } while (0)

#define BANDWIDTH(BYTES, MSEC) \
  ((double)(BYTES)) / ((MSEC) * 1e6);
#endif /* __UTEST_UTEST_HPP__ */

