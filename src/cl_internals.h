/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef __CL_INTERNALS_H__
#define __CL_INTERNALS_H__

/* We put a header to identify each object. This will make the programmer life
 * easy if objects are wrongly used in the API
 */
#define CL_MAGIC_KERNEL_HEADER    0x1234567890abcdefLL
#define CL_MAGIC_CONTEXT_HEADER   0x0ab123456789cdefLL
#define CL_MAGIC_PROGRAM_HEADER   0x34560ab12789cdefLL
#define CL_MAGIC_QUEUE_HEADER     0x83650a12b79ce4dfLL
#define CL_MAGIC_SAMPLER_HEADER   0x686a0ecba79ce33fLL
#define CL_MAGIC_EVENT_HEADER     0x8324a9c810ebf90fLL
#define CL_MAGIC_MEM_HEADER       0x381a27b9ce6504dfLL
#define CL_MAGIC_DEAD_HEADER      0xdeaddeaddeaddeadLL
#define CL_MAGIC_ACCELERATOR_INTEL_HEADER   0x7c6a08c9a7ac3e3fLL

#endif /* __CL_INTERNALS_H__ */

