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
 * \file utest_context.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Test basic functionalities of the context
 */
#include "utest/utest.hpp"
#include "ir/context.hpp"
#include "ir/unit.hpp"

namespace gbe
{
  using namespace ir;
  static void emptyFunction(void) {
    Unit unit;
    Context ctx(unit);
    ctx.startFunction("hop");
    ctx.endFunction();
  }
  static void oneInstruction(void) {
    Unit unit;
    Context ctx(unit);
    ctx.startFunction("hop");
      const Register reg0 = ctx.reg(FAMILY_DWORD);
      const Register reg1 = ctx.reg(FAMILY_DWORD);
      const Register reg2 = ctx.reg(FAMILY_DWORD);
      ctx.MAD(TYPE_FLOAT, reg0, reg0, reg1, reg2);
    ctx.endFunction();
  }
  static void noStartFunction(void) {
    Unit unit;
    Context ctx(unit);
    ctx.endFunction();
  }
  static void recursiveDefinition(void) {
    Unit unit;
    Context ctx(unit);
    ctx.startFunction("hop");
      const Register reg0 = ctx.reg(FAMILY_DWORD);
      const Register reg1 = ctx.reg(FAMILY_DWORD);
      const Register reg2 = ctx.reg(FAMILY_DWORD);
      ctx.MAD(TYPE_FLOAT, reg0, reg0, reg1, reg2);
      ctx.startFunction("bip");
        const LabelIndex label = ctx.label();
        ctx.LABEL(label);
        ctx.BRA(label);
      ctx.endFunction();
    ctx.endFunction();
#if 0
    std::cout << *unit.getFunction("hop") << std::endl;
    std::cout << *unit.getFunction("bip") << std::endl;
#endif
  }
  static void labelUsedTwice(void) {
    Unit unit;
    Context ctx(unit);
    ctx.startFunction("hop");
      const LabelIndex label = ctx.label();
      ctx.LABEL(label);
      ctx.LABEL(label);
    ctx.endFunction();
  }
  static void labelNotDefined(void) {
    Unit unit;
    Context ctx(unit);
    ctx.startFunction("hop");
      const LabelIndex label = ctx.label();
      ctx.BRA(label);
    ctx.endFunction();
  }
} /* namespace gbe */

static void utestContext(void)
{
  UTEST_EXPECT_SUCCESS(gbe::emptyFunction());
  UTEST_EXPECT_SUCCESS(gbe::oneInstruction());
  UTEST_EXPECT_FAILED(gbe::noStartFunction());
  UTEST_EXPECT_SUCCESS(gbe::recursiveDefinition());
  UTEST_EXPECT_FAILED(gbe::labelUsedTwice());
  UTEST_EXPECT_FAILED(gbe::labelNotDefined());
}

UTEST_REGISTER(utestContext)



