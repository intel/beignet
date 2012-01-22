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

#include "utest/utest.hpp"
#include "sys/command.hpp"
#include "sys/console.hpp"
#include "sys/logging.hpp"
#include "sys/windowing.hpp"
#include "sys/script.hpp"
#include <string>

namespace pf
{
  /*! Output everything in the terminal */
  class UTestConsoleDisplay : public ConsoleDisplay
  {
  public:
    UTestConsoleDisplay (void) {
      last = lastBlink = getSeconds();
      cursor = 0;
    }
    virtual void line(Console &console, const std::string &line) {
      const double curr = getSeconds();
      std::string patched = line;
      if (curr - lastBlink > .5) {
        cursor ^= 1;
        lastBlink = curr;
      }
      const uint32 pos = console.cursorPosition();
      if (cursor) {
        if (pos >= patched.size())
          patched.push_back('_');
        else
          patched[pos] = '_';
      }
      if (curr - last > 0.02) {
        std::cout << '\r' << "> " << patched;
        for (int i = 0; i < 80; ++i) std::cout << ' ';
        std::cout << '\r';
        fflush(stdout);
        last = curr;
      }
    }
    virtual void out(Console &console, const std::string &str) {
      std::cout << str << std::endl;
    }
    double last;
    double lastBlink;
    uint32 cursor;
  };
} /* namespace pf */

void utest_console(void)
{
  using namespace pf;
  WinOpen(640, 480);
  ScriptSystem *scriptSystem = LuaScriptSystemCreate();
  CommandSystemStart(*scriptSystem);
  UTestConsoleDisplay *display = GBE_NEW(UTestConsoleDisplay);
  Console *console = ConsoleNew(*scriptSystem, *display);
  console->addCompletion("while");
  console->addCompletion("whilewhile");
  for (;;) {
    Ref<InputControl> input = GBE_NEW(InputControl);
    input->processEvents();
    if (input->getKey(GBE_KEY_ASCII_ESC))
      break;
    console->update(*input);
    WinSwapBuffers();
  }
  CommandSystemEnd();
  GBE_DELETE(console);
  GBE_DELETE(scriptSystem);
  GBE_DELETE(display);
  WinClose();
}

UTEST_REGISTER(utest_console);

