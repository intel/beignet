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
 * \file cvar.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Quake like console variable system. Just use the environment variables from
 * the console to change their value
 */

#ifndef __GBE_CVAR_HPP__
#define __GBE_CVAR_HPP__

#include "sys/platform.hpp"

namespace gbe
{
  /*! A CVar is either a float, an integer or a string value. CVarInit is only
   *  here to set the global variable in pre-main
   */
  class CVarInit
  {
  public:
    enum {
      STRING = 0,
      INTEGER = 1,
      FLOAT = 2
    };
    /*! Build a CVar from an integer environment variable */
    explicit CVarInit(const char *name, int32_t *addr, int32_t imin, int32_t i, int32_t imax);
    /*! Build a CVar from a float environment variable */
    explicit CVarInit(const char *name, float *addr, float fmin, float f, float fmax);
    /*! Build a CVar from a string environment variable */
    explicit CVarInit(const char *name, std::string *str, const std::string &v);
    int varType;      //!< STRING, INTEGER or FLOAT
    std::string *str; //!< string variable
    union {
      struct { int32_t min, *curr, max; } i; //!< integer variables with bounds
      struct { float   min, *curr, max; } f; //!< float variables with bounds
    };
  };
} /* namespace gbe */

/*! Declare an integer console variable */
#define IVAR(NAME, MIN, CURR, MAX) \
  int32_t NAME; \
  static gbe::CVarInit __CVAR##NAME##__LINE__##__(#NAME, &NAME, int32_t(MIN), int32_t(CURR), int32_t(MAX));

/*! Declare a float console variable */
#define FVAR(NAME, MIN, CURR, MAX) \
  float NAME; \
  static gbe::CVarInit __CVAR##NAME##__LINE__##__(#NAME, &NAME, float(MIN), float(CURR), float(MAX));

/*! Declare a string console variable */
#define SVAR(NAME, STR) \
  std::string NAME; \
  static gbe::CVarInit __CVAR##NAME##__LINE__##__(#NAME, &NAME, STR);

/*! Declare a Boolean variable (just an integer in {0,1}) */
#define BVAR(NAME, CURR) IVAR(NAME, 0, CURR ? 1 : 0, 1)

#endif /* __GBE_CVAR_HPP__ */

