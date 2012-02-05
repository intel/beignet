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

#ifndef __GBE_UTEST_HPP__
#define __GBE_UTEST_HPP__

#include <vector>

namespace gbe
{
  /*! Quick and dirty Unit test system with registration */
  struct UTest
  {
    /*! A unit test function to run */
    typedef void (*Function) (void);
    /*! Empty test */
    UTest(void);
    /*! Build a new unit test and append it to the unit test list */
    UTest(Function fn, const char *name);
    /*! Function to execute */
    Function fn;
    /*! Name of the test */
    const char *name;
    /*! The tests that are registered */
    static std::vector<UTest> *utestList;
    /*! Run the test with the given name */
    static void run(const char *name);
    /*! Run all the tests */
    static void runAll(void);
  };
} /* namespace gbe */

/*! Register a new unit test */
#define UTEST_REGISTER(FN) static const gbe::UTest __##NAME##__(FN, #FN);

#endif /* __GBE_UTEST_HPP__ */

