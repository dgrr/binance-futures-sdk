#ifndef BINANCE_COMMON_HPP
#define BINANCE_COMMON_HPP

#include <simdjson.h>  // really_inline here

#include <boost/asio/io_context.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <string>

#ifndef really_inline
#define really_inline __attribute__((always_inline)) inline
#endif

#ifdef BINANCE_USE_STRING_VIEW
using string_type = std::string_view;
#else
using string_type = std::string;
#endif

namespace binance
{
using io_context   = boost::asio::io_context;
using boost_error  = boost::system::error_code;
using time_point_t = std::chrono::time_point<std::chrono::system_clock>;
}  // namespace binance

#ifndef nano_epoch
#define nano_epoch()                                      \
  std::chrono::time_point_cast<std::chrono::nanoseconds>( \
      std::chrono::system_clock::now())                   \
      .time_since_epoch()                                 \
      .count()
#endif

#ifndef string_nano_epoch
#define string_nano_epoch() std::to_string(static_cast<long int>(nano_epoch()))
#endif

#ifndef milli_epoch
#define milli_epoch()                                      \
  std::chrono::time_point_cast<std::chrono::milliseconds>( \
      std::chrono::system_clock::now())                    \
      .time_since_epoch()                                  \
      .count()
#endif

#ifndef string_milli_epoch
#define string_milli_epoch() \
  std::to_string(static_cast<long int>(milli_epoch()))
#endif

#endif
