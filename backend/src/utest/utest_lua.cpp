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

#include "sys/script.hpp"
#include "sys/command.hpp"
#include "sys/logging.hpp"
#include "sys/tasking.hpp"
#include "utest/utest.hpp"

using namespace pf;

VARI(coucou, 0, 2, 3, "coucou");
VARS(player0, "ben", "player name");

#define _RUN_SCRIPT(STR, RUN_MODE) do {\
  ScriptStatus status;\
  scriptSystem->RUN_MODE(STR, status);\
  if (!status.success) GBE_ERROR(status.msg);\
} while (0)
#define RUN(STR) _RUN_SCRIPT(STR,run)
#define RUN_NON_PROTECTED(STR) _RUN_SCRIPT(STR,runNonProtected)

void utest_lua(void)
{
  ScriptSystem *scriptSystem = LuaScriptSystemCreate();
  ScriptStatus status;
  scriptSystem->run("local x = 0", status);
  CommandSystemStart(*scriptSystem);

  // Run some code. This may modify console variables
  RUN("cv.coucou = 1");
  RUN_NON_PROTECTED("print(pf.cv.coucou)");
  RUN("cv.player0 = \"hop\"");
  RUN_NON_PROTECTED("print(pf.cv.player0)");
  if (coucou() == 1) GBE_MSG("coucou is equal to 1");

  CommandSystemEnd();
  GBE_DELETE(scriptSystem);
}

UTEST_REGISTER(utest_lua)

