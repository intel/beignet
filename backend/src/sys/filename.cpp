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

#include "sys/platform.hpp"
#include "sys/filename.hpp"

namespace pf
{
#ifdef __WIN32__
  const char path_sep = '\\';
#else
  const char path_sep = '/';
#endif

  /*! little helper to not depend on math */
  static size_t maxInt(size_t a, size_t b) { return index_t(a) < index_t(b) ? b : a; }

  /*! create an empty filename */
  FileName::FileName () {}

  /*! create a valid filename from a string */
  FileName::FileName (const char* in) {
    filename = in;
    for (size_t i=0; i<filename.size(); i++)
      if (filename[i] == '\\' || filename[i] == '/')
        filename[i] = path_sep;
    while (!filename.empty() && filename[filename.size()-1] == path_sep)
      filename.resize(filename.size()-1);
  }

  /*! create a valid filename from a string */
  FileName::FileName (const std::string& in) {
    filename = in;
    for (size_t i=0; i<filename.size(); i++)
      if (filename[i] == '\\' || filename[i] == '/')
        filename[i] = path_sep;
    while (!filename.empty() && filename[filename.size()-1] == path_sep)
      filename.resize(filename.size()-1);
  }

  /*! returns the path */
  FileName FileName::path() const {
    size_t pos = maxInt(filename.find_last_of('\\'),filename.find_last_of('/'));
    if (pos == std::string::npos) return FileName();
    return filename.substr(0,pos);
  }

  /*! returns the basename */
  std::string FileName::base() const {
    size_t pos = maxInt(filename.find_last_of('\\'),filename.find_last_of('/'));
    if (pos == std::string::npos) return filename;
    return filename.substr(pos+1);
  }

  /*! returns the extension */
  std::string FileName::ext() const {
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return "";
    return filename.substr(pos+1);
  }

  /*! returns the basename without extension */
  std::string FileName::name() const {
    size_t start = maxInt(filename.find_last_of('\\'),filename.find_last_of('/')) + 1;
    if (start == std::string::npos) start = 0;
    size_t end = filename.find_last_of('.');
    if (end == std::string::npos || end < start) end = filename.size();
    return filename.substr(start, end - start);
  }

  /*! replaces the extension */
  FileName FileName::setExt(const std::string& ext) const {
    size_t start = maxInt(filename.find_last_of('\\'),filename.find_last_of('/')) + 1;
    if (start == std::string::npos) start = 0;
    size_t end = filename.find_last_of('.');
    if (end == std::string::npos || end < start) return FileName(filename+ext);
    return FileName(filename.substr(0,end)+ext);
  }

  /*! adds the extension */
  FileName FileName::addExt(const std::string& ext) const {
    return FileName(filename+ext);
  }

  /*! concatenates two filenames to this/other */
  FileName FileName::operator +( const FileName& other ) const {
    if (filename == "") return FileName(other);
    else return FileName(filename + path_sep + other.filename);
  }

  /*! concatenates two filenames to this/other */
  FileName FileName::operator +( const std::string& other ) const {
    return operator+(FileName(other));
  }

  /*! removes the base from a filename (if possible) */
  FileName FileName::operator -( const FileName& base ) const {
    size_t pos = filename.find_first_of(base);
    if (pos == std::string::npos) return *this;
    return FileName(filename.substr(pos+1));
  }

  /*! output operator */
  std::ostream& operator<<(std::ostream& cout, const FileName& filename) {
    return cout << filename.filename;
  }
}
