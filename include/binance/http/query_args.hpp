#ifndef BINANCE_QUERY_ARGS_HPP
#define BINANCE_QUERY_ARGS_HPP

#include <binance/common.hpp>
#include <boost/json.hpp>
#include <boost/variant2/variant.hpp>
#include <string>
#include <utility>
#include <vector>

namespace binance
{
namespace http
{
struct args_visitor_to_string
{
  // TODO: Avoid the copy constructor here
  std::string operator()(const std::string& s) const
  {
    return s;
  }
  std::string operator()(size_t n) const
  {
    return std::to_string(n);
  }
  std::string operator()(bool b) const
  {
    return b ? "true" : "false";
  }
  std::string operator()(int64_t n) const
  {
    return std::to_string(n);
  }
  std::string operator()(double d) const
  {
    return std::to_string(d);
  }
};

class query_args
{
public:
  using key_value =
      std::pair<std::string, boost::variant2::variant<std::string, size_t,
                                                      int64_t, bool, double>>;

  explicit query_args() = default;
  explicit query_args(std::initializer_list<key_value> args)
  {
    for (const key_value& kv : args)
    {
      if (is_second_empty(kv))
        continue;
      args_.push_back(kv);
    }
  }
  explicit query_args(const query_args& args)
  {
    args_ = args.args_;
  }
  operator bool() const
  {
    return !args_.empty();
  }
  really_inline const std::vector<key_value> args() const
  {
    return args_;
  }
  really_inline bool get(const std::string& key, std::string& value) const
  {
    for (auto& kv : args_)
    {
      if (kv.first == key)
      {
        value = boost::variant2::get<0>(kv.second);
        return true;
      }
    }
    return false;
  }
  really_inline bool get(const std::string& key, size_t& value) const
  {
    for (auto& kv : args_)
    {
      if (kv.first == key)
      {
        value = boost::variant2::get<1>(kv.second);
        return true;
      }
    }
    return false;
  }
  really_inline bool get(const std::string& key, int64_t& value) const
  {
    for (auto& kv : args_)
    {
      if (kv.first == key)
      {
        value = boost::variant2::get<2>(kv.second);
        return true;
      }
    }
    return false;
  }
  really_inline bool get(const std::string& key, bool& value) const
  {
    for (auto& kv : args_)
    {
      if (kv.first == key)
      {
        value = boost::variant2::get<3>(kv.second);
        return true;
      }
    }
    return false;
  }
  really_inline bool get(const std::string& key, double& value) const
  {
    for (auto& kv : args_)
    {
      if (kv.first == key)
      {
        value = boost::variant2::get<4>(kv.second);
        return true;
      }
    }
    return false;
  }
  really_inline void insert_kv(key_value&& kv)
  {
    if (is_second_empty(kv))
      return;
    for (auto& v : args_)
    {
      if (v.first == kv.first)
      {
        v.second = kv.second;
        return;
      }
    }
    args_.push_back(kv);
  }

protected:
  std::vector<key_value> args_;

private:
  inline bool is_second_empty(const key_value& kv)
  {
    return kv.second.index() == 0 && boost::variant2::get<0>(kv.second).empty();
  }
};
}  // namespace http
}  // namespace binance

#endif
