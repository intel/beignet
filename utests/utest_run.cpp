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
#include "utest.hpp"

int main(int argc, char *argv[])
{
  if (argc >= 2)
    for (int i = 1; i < argc; ++i)
      UTest::run(argv[i]);
  else
    UTest::runAll();
}

