#ifndef BINANCE_AUTH_HPP
#define BINANCE_AUTH_HPP

#include <string>

namespace binance
{
struct auth_opts
{
private:
  bool empty_;

public:
  std::string key;
  std::string secret;

  auth_opts()
      : empty_(true)
  {
  }
  auth_opts(const std::string& key, const std::string& secret)
      : key(key)
      , secret(secret)
  {
    empty_ = key.empty() || secret.empty();
  }
  operator bool() const
  {
    return !empty_;
  }
};
}  // namespace binance

#endif