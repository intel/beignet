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

/**
 * \file assert.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __OCL_ASSERT_HPP__
#define __OCL_ASSERT_HPP__

/*! To ensure that condition truth. Optional message is supported */
void onFailedAssertion(const char *msg, const char *file, const char *fn, int line);

#define OCL_ASSERT(EXPR) \
  do { \
    if (!(EXPR)) \
      onFailedAssertion(#EXPR, __FILE__, __FUNCTION__, __LINE__); \
  } while (0)

#define OCL_ASSERTM(EXPR, MSG) \
  do { \
    if (!(EXPR)) \
      onFailedAssertion(MSG, __FILE__, __FUNCTION__, __LINE__); \
  } while (0)

#endif /* __OCL_ASSERT_HPP__ */

