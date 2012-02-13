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
 * \file blob.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Compile the complete project from one file. This allows pretty aggresive
 * optimization from the compiler and decreases the binary size
 */

#include "sys/assert.cpp"
#include "sys/string.cpp"
#include "sys/alloc.cpp"
#include "sys/sysinfo.cpp"
#include "sys/mutex.cpp"
#include "sys/condition.cpp"
#include "sys/platform.cpp"
#include "ir/context.cpp"
#include "ir/unit.cpp"
#include "ir/constant.cpp"
#include "ir/instruction.cpp"
#include "ir/register.cpp"
#include "ir/function.cpp"

#if GBE_COMPILE_UTEST

#endif /* GBE_COMPILE_UTEST */

