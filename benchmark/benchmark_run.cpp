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
#include <getopt.h>

static const char *shortopts = "c:lanh";
struct option longopts[] = {
{"casename", required_argument, NULL, 'c'},
{"list", no_argument, NULL, 'l'},
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
    -l           : list all the available case name\n\
    -a           : run all test cases\n\
    -n           : run all test cases without known issue (default option)\n\
    -h           : display this usage\n\
\
    "<< std::endl;
}

int main(int argc, char *argv[])
{

  int c = 0;
  cl_ocl_init();

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

      case 'l':
        UTest::listAllCases();
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

  cl_ocl_destroy();
}
