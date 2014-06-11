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
 * \file gen_mesa_disasm.h
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * To decode and print one Gen ISA instruction. The code is directly taken
 * from Mesa
 */

#ifndef __GBE_GEN_MESA_DISASM_H__
#define __GBE_GEN_MESA_DISASM_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int gen_disasm(FILE *file, const void *opaque_insn, uint32_t deviceID, uint32_t compacted);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GBE_GEN_MESA_DISASM_H__ */


