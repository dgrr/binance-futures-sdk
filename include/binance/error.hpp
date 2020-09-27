#ifndef BINANCE_ERROR_HPP
#define BINANCE_ERROR_HPP

#include <boost/system/error_code.hpp>
#include <string>

namespace binance
{
class error
{
  int ec_;
  std::string ec_s_;

public:
  error()
      : ec_(0)
  {
  }
  error(boost::system::error_code ec)
  {
    ec_   = -1;
    ec_s_ = ec.message();
  }
  error(unsigned int code, const std::string& body)
  {
    ec_ = int(code);
    ec_s_ = body;
  }
  operator bool() const
  {
    return ec_ != 0;
  }
  error& operator=(int code)
  {
    ec_ = code;
    return *this;
  }
  error& operator=(const std::string& s)
  {
    ec_s_ = s;
    return *this;
  }
  const int code() const
  {
    return ec_;
  }
  const std::string& to_string() const
  {
    return ec_s_;
  }
  friend std::ostream& operator<<(std::ostream& os, const error& ec);
};

std::ostream& operator<<(std::ostream& os, const error& ec)
{
  os << ec.ec_ << ": " << ec.ec_s_;
  return os;
}
}  // namespace binance


#endif