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

#include "string"
namespace gbe {
std::string simulator_str = 
"/*\n"
" * Copyright 2012 Intel Corporation\n"
" *\n"
" * Permission is hereby granted, free of charge, to any person obtaining a\n"
" * copy of this software and associated documentation files (the \"Software\"),\n"
" * to deal in the Software without restriction, including without limitation\n"
" * the rights to use, copy, modify, merge, publish, distribute, sublicense,\n"
" * and/or sell copies of the Software, and to permit persons to whom the\n"
" * Software is furnished to do so, subject to the following conditions:\n"
" *\n"
" * The above copyright notice and this permission notice (including the next\n"
" * paragraph) shall be included in all copies or substantial portions of the\n"
" * Software.\n"
" *\n"
" * THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
" * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
" * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL\n"
" * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
" * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
" * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER\n"
" * DEALINGS IN THE SOFTWARE.\n"
" */\n"
"\n"
"/**\n"
" * \file simulator.h\n"
" * \author Benjamin Segovia <benjamin.segovia@intel.com>\n"
" *\n"
" * C interface for the gen simulator\n"
" */\n"
"\n"
"#ifndef __GBE_SIMULATOR_H__\n"
"#define __GBE_SIMULATOR_H__\n"
"\n"
"#ifdef __cplusplus\n"
"extern \"C\" {\n"
"#endif /* __cplusplus */\n"
"\n"
"/* Gen simulator that runs the c++ produced by the back-end */\n"
"typedef struct _gbe_simulator *gbe_simulator;\n"
"/* Return the base address of the global / constant memory space */\n"
"typedef void *(sim_get_base_address_cb)(gbe_simulator);\n"
"/* Set the base address of the global / constant memory space */\n"
"typedef void (sim_set_base_address_cb)(gbe_simulator, void*);\n"
"/* Set the base address of the constant buffer */\n"
"typedef void *(sim_get_curbe_address_cb)(gbe_simulator);\n"
"/* Set the base address of the global / constant memory space */\n"
"typedef void (sim_set_curbe_address_cb)(gbe_simulator, void*);\n"
"struct _gbe_simulator {\n"
"  sim_set_base_address_cb *set_base_address;\n"
"  sim_get_base_address_cb *get_base_address;\n"
"  sim_set_curbe_address_cb *set_curbe_address;\n"
"  sim_get_curbe_address_cb *get_curbe_address;\n"
"};\n"
"\n"
"#ifdef __cplusplus\n"
"}\n"
"#endif /* __cplusplus */\n"
"\n"
"#endif /* __GBE_SIMULATOR_H__ */\n"
"\n"
;
}

