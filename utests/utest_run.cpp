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
 * \file utest_run.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Just run the unit tests. The user can possibly provides the subset of it
 */
#include "utest_helper.hpp"
#include "utest_exception.hpp"
#include <iostream>
#include <string.h>
#include <getopt.h>

static const char *shortopts = "c:j:l::anh";
struct option longopts[] = {
{"casename", required_argument, NULL, 'c'},
{"jobs", required_argument, NULL, 'j'},
{"list", optional_argument, NULL, 'l'},
{"all", no_argument, NULL, 'a'},
{"allnoissue", no_argument, NULL, 'n'},
{"help", no_argument, NULL, 'h'},
{0, 0, 0, 0},
};

void usage()
{
    std::cout << "\
Usage:\n\
  ./utest_run <option>\n\
\n\
  option:\n\
    -c <casename>: run sub-case named 'casename'\n\
    -j <number>  : specifies the 'number' of jobs (multi-thread)\n\
    -l <a/i>     : list case name that can run(a for all case, i for case with issue)\n\
    -a           : run all test cases\n\
    -n           : run all test cases without known issue (default option)\n\
    -h           : display this usage\n\
\
    "<< std::endl;
}

int main(int argc, char *argv[])
{

  int c = 0;
  if (cl_ocl_init() != CL_SUCCESS) {
    fprintf(stderr, "Failed to initialize cl device.\n");
    goto clean;
  }

  c = getopt_long (argc, argv, shortopts, longopts, NULL);

  if (argc == 1)
    c = 'n';
  if (argc == 2 && c < 1 ){
    c = 'c';
    optarg = argv[1];
  }

  do {
    switch (c)
    {
      case 'c':
        try {
          UTest::run(optarg);
        }
        catch (Exception e){
          std::cout << "  " << e.what() << "    [SUCCESS]" << std::endl;
        }

        break;

      case 'j':
        try {
#if defined(__ANDROID__)
          std::cout << "Do not support multithread in android, use single thread instead." << std::endl;
          UTest::run(optarg);
#else
          UTest::runMultiThread(optarg);
#endif
        }
        catch (Exception e){
          std::cout << "  " << e.what() << "    [SUCCESS]" << std::endl;
        }

        break;

      case 'l':
        if (optarg == NULL)
          UTest::listCasesCanRun();
        else if (strcmp(optarg,"a") == 0)
          UTest::listAllCases();
        else if (strcmp(optarg,"i") == 0)
          UTest::listCasesWithIssue();
        else {
          usage();
          exit(1);
        }
        break;

      case 'a':
        try {
          UTest::runAll();
        }
        catch (Exception e){
          std::cout << "  " << e.what() << "    [SUCCESS]" << std::endl;
        }

        break;

      case 'n':
        try {
          UTest::runAllNoIssue();
        }
        catch (Exception e){
          std::cout << "  " << e.what() << "    [SUCCESS]" << std::endl;
        }

        break;

      case 'b':
        try {
          UTest::runAllBenchMark();
        }
        catch (Exception e){
          std::cout << "  " << e.what() << "    [SUCCESS]" << std::endl;
        }

        break;


      case 'h':
      default:
        usage();
        exit(1);
    }
  } while ((c = getopt_long (argc, argv, shortopts, longopts, NULL)) != -1);

clean:
  cl_ocl_destroy();
  return 0;
}

