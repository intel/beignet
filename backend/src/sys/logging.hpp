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

#ifndef __PF_LOGGING_HPP__
#define __PF_LOGGING_HPP__

#include "sys/mutex.hpp"
#include "sys/platform.hpp"
#include <sstream>
#include <iomanip>

namespace pf
{
  class Logger;
  class LoggerBuffer;
  class LoggerStream;

  /*! A logger stream is one way to output a string. It can be a file,
   *  stdout, the in-game console and so on...
   */
  class LoggerStream
  {
  public:
    LoggerStream(void);
    virtual ~LoggerStream(void);
    virtual LoggerStream& operator<< (const std::string &str) = 0;
  private:
    friend class Logger;
    LoggerStream *next; //!< We chain the logger elements together
  };

  /*! Helper and proxy structures to display various information */
  static struct LoggerFlushTy { } loggerFlush MAYBE_UNUSED;
  struct LoggerInfo {
    INLINE LoggerInfo(const char *file, const char *function, int line) :
      file(file), function(function), line(line) {}
    const char *file, *function;
    int line;
    PF_CLASS(LoggerInfo);
  };

  /*! Used to lazily create strings from the user defined logs. When destroyed
   *  or flushed, it displays everything in one piece
   */
  class LoggerBuffer
  {
  public:
    template <typename T> LoggerBuffer& operator<< (const T &x) {
      ss << x;
      return *this;
    }
    LoggerBuffer& operator<< (LoggerFlushTy);
    LoggerBuffer& operator<< (const LoggerInfo&);
    /*! Output the info a nice format */
    LoggerBuffer& info(const char *file, const char *function, int line);
  private:
    friend class Logger;
    LoggerBuffer(void);
    Logger *logger;       //!< The logger that created this buffer
    std::stringstream ss; //!< Stores all the user strings
    PF_CLASS(LoggerBuffer);
  };

  /*! Central class to log anything from the engine */
  class Logger
  {
  public:
    Logger(void);
    ~Logger(void);
    /*! Output the string into all the attached streams */
    void output(const std::string &str);
    template <typename T> LoggerBuffer& operator<< (const T &x) {
      const uint32 bufferID = 0;
      LoggerBuffer &buffer = buffers[bufferID];
      buffer << "[" << "thread " << std::setw(2) << bufferID << "] ";
      buffer << "[" << std::setw(12) << std::fixed << getSeconds() - startTime << "s] " << x;
      return buffer;
    }
    void insert(LoggerStream &stream);
    void remove(LoggerStream &stream);
  private:
    MutexSys mutex;        //!< To insert / remove streams and output strings
    LoggerStream *streams; //!< All the output streams
    LoggerBuffer *buffers; //!< One buffer per thread
    double startTime;      //!< When the logger has been created
    PF_CLASS(Logger);
  };

  /*! We have one logger for the application */
  extern Logger *logger;

} /* namespace pf */

/*! Macros to handle logging information in the code */
#define PF_LOG_HERE LoggerInfo(__FILE__, __FUNCTION__, __LINE__)
#define PF_INFO " ######## " << PF_LOG_HERE

/*! Verbose macros: they add logging position and thread ID */
#define PF_WARNING_V(MSG) do {                                                \
  if (logger) *logger << "WARNING " << MSG << PF_INFO << "\n" << loggerFlush; \
} while (0)

#define PF_ERROR_V(MSG)   do {                                                \
  if (logger) *logger << "ERROR " << MSG << PF_INFO << "\n" << loggerFlush;   \
} while (0)

#define PF_MSG_V(MSG)     do {                                                \
  if (logger) *logger << MSG << PF_INFO << "\n" << loggerFlush;               \
} while (0)

/*! Regular macros: just the user message */
#define PF_WARNING(MSG) do {                                                  \
  if (logger) *logger << "WARNING " << MSG << "\n" << loggerFlush;            \
} while (0)

#define PF_ERROR(MSG)   do {                                                  \
  if (logger) *logger << "ERROR " << MSG << "\n" << loggerFlush;              \
} while (0)

#define PF_MSG(MSG)     do {                                                  \
  if (logger) *logger << MSG << "\n" << loggerFlush;                          \
} while (0)

#endif /* __PF_LOGGING_HPP__ */

