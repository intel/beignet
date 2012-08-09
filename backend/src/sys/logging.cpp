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

#include "sys/logging.hpp"
#include "sys/filename.hpp"

namespace pf
{
  LoggerStream::LoggerStream(void) : next(NULL) {}
  LoggerStream::~LoggerStream(void) {}

  LoggerBuffer::LoggerBuffer(void) : logger(NULL) {}

  LoggerBuffer& LoggerBuffer::operator<< (LoggerFlushTy) {
    logger->output(ss.str());
    ss.str("");
    return *this;
  }

  LoggerBuffer& LoggerBuffer::operator<< (const LoggerInfo &info) {
    FileName fileName(info.file);
    return *this << fileName.base() << " at " << info.function << " line " << info.line;
  }

  Logger::Logger(void) : streams(NULL) {
    const uint32 threadNum = 1;
    this->buffers = PF_NEW_ARRAY(LoggerBuffer, threadNum);
    for (uint32 i = 0; i < threadNum; ++i) this->buffers[i].logger = this;
    this->startTime = getSeconds();
  }

  Logger::~Logger(void) {
    FATAL_IF(streams != NULL, "Remove all streams before deleting the logger");
    PF_DELETE_ARRAY(buffers);
  }

  void Logger::output(const std::string &str) {
    Lock<MutexSys> lock(mutex);
    LoggerStream *stream = this->streams;
    while (stream) {
      *stream << str;
      stream = stream->next;
    }
  }

  void Logger::insert(LoggerStream &stream) {
    Lock<MutexSys> lock(mutex);
    stream.next = this->streams;
    this->streams = &stream;
  }

  void Logger::remove(LoggerStream &stream) {
    Lock<MutexSys> lock(mutex);
    LoggerStream *curr = this->streams;
    LoggerStream *pred = NULL;
    while (curr) {
      if (curr == &stream)
        break;
      pred = curr;
      curr = curr->next;
    }
    FATAL_IF (curr == NULL, "Unable to find the given stream");
    if (pred)
      pred->next = curr->next;
    else
      this->streams = curr->next;
  }

  Logger *logger = NULL;
} /* namespace pf */

