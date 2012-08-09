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

#ifndef __PF_FILENAME_HPP__
#define __PF_FILENAME_HPP__

#include "platform.hpp"
#include <string>
#include <cstdio>

namespace pf
{
  /*! Convenience class for handling file names and paths. */
  class FileName
  {
  public:
    /*! create an empty filename */
    FileName ();
    /*! create a valid filename from a string */
    FileName (const char* filename);
    /*! create a valid filename from a string */
    FileName (const std::string& filename);
    /*! auto convert into a string */
    operator std::string() const { return filename; }
    /*! returns a string of the filename */
    const std::string str() const { return filename; }
    /*! returns a c-string of the filename */
    const char* c_str() const { return filename.c_str(); }
    /*! returns the path of a filename */
    FileName path() const;
    /*! returns the file of a filename  */
    std::string base() const;
    /*! returns the base of a filename without extension */
    std::string name() const;
    /*! returns the file extension */
    std::string ext() const;
    /*! replaces the file extension */
    FileName setExt(const std::string& ext = "") const;
    /*! adds file extension */
    FileName addExt(const std::string& ext = "") const;
    /*! concatenates two filenames to this/other */
    FileName operator +( const FileName& other ) const;
    /*! concatenates two filenames to this/other */
    FileName operator +( const std::string& other ) const;
    /*! removes the base from a filename (if possible) */
    FileName operator -( const FileName& base ) const;
    /*! output operator */
    friend std::ostream& operator<<(std::ostream& cout, const FileName& filename);
  private:
    std::string filename;
    PF_CLASS(FileName);
  };
}

#endif /* __PF_FILENAME_HPP__ */

