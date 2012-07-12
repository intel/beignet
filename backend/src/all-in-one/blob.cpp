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

#include "ocl_stdlib_str.cpp"
#include "sys/assert.cpp"
#include "sys/string.cpp"
#include "sys/alloc.cpp"
#include "sys/sysinfo.cpp"
#include "sys/mutex.cpp"
#include "sys/condition.cpp"
#include "sys/platform.cpp"
#include "sys/cvar.cpp"
#include "ir/context.cpp"
#include "ir/type.cpp"
#include "ir/unit.cpp"
#include "ir/constant.cpp"
#include "ir/instruction.cpp"
#include "ir/register.cpp"
#include "ir/function.cpp"
#include "ir/liveness.cpp"
#include "ir/value.cpp"
#include "ir/lowering.cpp"
#include "ir/profile.cpp"
#include "backend/context.cpp"
#include "backend/program.cpp"
#include "backend/sim_context.cpp"
#include "backend/sim_program.cpp"
#include "backend/sim/simulator_str.cpp"
#include "backend/sim/sim_vector_str.cpp"
#include "backend/gen_insn_selection.cpp"
#include "backend/gen_reg_allocation.cpp"
#include "backend/gen_context.cpp"
#include "backend/gen_program.cpp"
#include "backend/gen_encoder.cpp"

