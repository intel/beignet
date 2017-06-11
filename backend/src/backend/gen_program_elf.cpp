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
 */
#include "ocl_common_defines.h"
#include "elfio/elfio.hpp"
#include "backend/gen_program.hpp"
#include "backend/gen_program.hpp"
#include <algorithm>
#include <sstream>
#include <streambuf>

namespace gbe
{
/* The elf writer need to make sure seekp function work, so sstream
   can not work, and we do not want the fostream to generate the real
   file. We just want to keep the elf image in the memory. Implement
   a simple streambuf write only class here. */
class wmemstreambuf : public std::streambuf
{
public:
  wmemstreambuf(size_t size) : max_writed(0)
  {
    buf_ = static_cast<char *>(::malloc(size));
    memset(buf_, 0, size);
    buf_size_ = size;
    setbuf(buf_, buf_size_);
  }
  ~wmemstreambuf()
  {
    if (buf_)
      ::free(buf_);
  }

  char *getcontent(size_t &total_sz)
  {
    total_sz = max_writed;
    return buf_;
  }

protected:
  char *buf_;
  std::streamsize buf_size_;
  std::streamsize max_writed;

  virtual std::streambuf *setbuf(char *s, std::streamsize n)
  {
    auto const begin(s);
    auto const end(s + n);
    setp(begin, end);
    return this;
  }

  virtual std::streampos seekpos(std::streampos pos,
                                 std::ios_base::openmode which =
                                   ::std::ios_base::in | ::std::ios_base::out)
  {
    if (which != std::ios_base::out) {
      assert(0);
      return pos_type(off_type(-1));
    }

    if (pos >= epptr() - pbase()) {
      auto old_size = buf_size_;
      while (buf_size_ < pos) {
        buf_size_ *= 2;
      }

      buf_ = static_cast<char *>(::realloc(buf_, buf_size_));
      memset(buf_ + old_size, 0, buf_size_ - old_size);
      setbuf(buf_, buf_size_);
    } else {
      setp(pbase(), epptr());
    }

    pbump(pos);
    return pos;
  }

  virtual int sync() { return 0; }
  virtual int overflow(int c) { return c; };

  virtual std::streamsize xsgetn(const char *s, std::streamsize count)
  {
    assert(0);
    return traits_type::eof();
  }

  virtual std::streamsize xsputn(const char *s, std::streamsize const count)
  {
    if (epptr() - pptr() < count) {
      auto old_pos = pptr() - pbase();
      while (buf_size_ < (pptr() - pbase()) + count) {
        buf_size_ *= 2;
      }
      buf_ = static_cast<char *>(::realloc(buf_, buf_size_));
      memset(buf_ + old_pos, 0, buf_size_ - old_pos);
      setbuf(buf_, buf_size_);
      pbump(old_pos);
    }

    std::memcpy(pptr(), s, count);
    if (pptr() - pbase() + count > max_writed)
      max_writed = pptr() - pbase() + count;

    pbump(count);

    return count;
  }
};
} /* namespace gbe */
