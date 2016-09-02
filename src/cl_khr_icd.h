/* 
 * Copyright © 2013 Simon Richter
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
 */
#ifndef __CL_KHR_ICD_H__
#define __CL_KHR_ICD_H__

#ifdef HAS_OCLIcd

#define SET_ICD(dispatch) \
  dispatch = &cl_khr_icd_dispatch;
#define DEFINE_ICD(member) struct _cl_icd_dispatch const *member;

extern struct _cl_icd_dispatch const cl_khr_icd_dispatch;
#else
#define SET_ICD(dispatch)
#define DEFINE_ICD(member)
#endif

#endif
