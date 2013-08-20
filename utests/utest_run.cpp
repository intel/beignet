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
 * \file utest_run.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Just run the unit tests. The user can possibly provides the subset of it
 */
#include "utest_helper.hpp"
#include "utest_exception.hpp"
#include <string.h>
#include <iostream>

int main(int argc, char *argv[])
{
  try {
    if (argc == 2 && !strcmp(argv[1], "--list")) {
      UTest::listAll();
      return 0;
    }

    cl_ocl_init();
    if (argc >= 2)
      for (int i = 1; i < argc; ++i)
        UTest::run(argv[i]);
    else
      UTest::runAll();
    cl_ocl_destroy();
  } catch (Exception e) {
      std::cout << "  " << e.what() << "    [SUCCESS]" << std::endl;
  }
}

