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
 * \file exception.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_EXCEPTION_HPP__
#define __GBE_EXCEPTION_HPP__

#if GBE_COMPILE_UTESTS

#include <exception>
#include <string>

namespace gbe
{
  /*! Exception are only used while using unit tests */
  class Exception : public std::exception
  {
  public:
    Exception(const std::string &msg) throw() : msg(msg) {}
    Exception(const Exception &other) throw() : msg(other.msg) {}
    ~Exception(void) throw() {}
    Exception &operator= (const Exception &other) throw() {
      this->msg = other.msg;
      return *this;
    }
    const char *what(void) const throw() { return msg.c_str(); }
  private:
    std::string msg; //!< String message
  };

} /* namespace gbe */

#endif /* GBE_COMPILE_UTESTS */
#endif /* __GBE_EXCEPTION_HPP__ */

