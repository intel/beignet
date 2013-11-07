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
 * \file program.h
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * C-like interface for the gen kernels and programs
 */

#ifndef __GBE_GEN_PROGRAM_H__
#define __GBE_GEN_PROGRAM_H__

#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>

/*! This will make the compiler output Gen ISA code */
extern void genSetupCallBacks(void);

#endif /* __GBE_GEN_PROGRAM_H__ */

