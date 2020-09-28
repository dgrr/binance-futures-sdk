#ifndef BINANCE_JSON_HPP
#define BINANCE_JSON_HPP

#include <immintrin.h>
#include <simdjson.h>

#include <binance/common.hpp>
#include <binance/conv.hpp>
#include <binance/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <chrono>
#include <string>
#include <string_view>

namespace binance
{
namespace json
{
class parser
{
  simdjson::dom::parser parser_;
  simdjson::dom::element root_;

public:
  parser(size_t alloc_mem = 128 * 1024)
  {
    if (parser_.allocate(alloc_mem) != simdjson::error_code::SUCCESS)  // 128 KB
      throw std::bad_alloc{};
  }
  parser(parser&);
  parser(parser&&);
  inline parser& parse(const char* buffer, size_t buffer_size)
  {
    root_ = parser_.parse(buffer, buffer_size, false);
    return *this;
  }

  inline parser& parse(const std::string& buffer)
  {
    parse((const char*) &buffer[0], buffer.size());
    return *this;
  }

  inline parser& parse(const boost::beast::flat_buffer& buffer)
  {
    const char* body = (const char*) buffer.data().data();
    parse(body, buffer.size());
    return *this;
  }

  really_inline simdjson::dom::element& root()
  {
    return root_;
  }
};

using object = simdjson::dom::object;
using array  = simdjson::dom::array;
using value  = simdjson::dom::element;

really_inline int64_t to_int(const value& e)
{
  int64_t val = 0;
  if (e.is<int64_t>())
    val = e;
  else
  {
    std::string_view S = e.get<std::string_view>();
    const char* s      = S.data();
    val                = binance::conv::parse_int(&s[0], S.size());
  }

  return val;
}
// TODO: Use SIMD
really_inline double to_float(const value& e)
{
  if (e.is<double>())
    return e.get<double>();
  const char* s = e.get<const char*>();
  return binance::conv::parse_float(s);
}

template<typename ChronoScale = std::chrono::milliseconds>
really_inline time_point_t to_time(const value& e)
{
  int64_t n = to_int(e);
  auto d    = ChronoScale(n);
  return time_point_t(d);
}

// json to primitive types
really_inline void value_to(const value& v, double& d)
{
  d = to_float(v);
}

really_inline void value_to(const value& v, int64_t& i)
{
  i = to_int(v);
}

really_inline void value_to(const value& v, int& i)
{
  i = int(to_int(v));
}

really_inline void value_to(const value& v, std::string& s)
{
  s = v;
}

really_inline void value_to(const value& v, std::string_view& s)
{
  s = v;
}

template<typename ChronoScale = std::chrono::milliseconds>
really_inline void value_to(const value& v, time_point_t& t)
{
  t = to_time<ChronoScale>(v);
}

/**
 * assign from json
 **/

really_inline void value_to(const object& jv, const char* key, json::value& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    v = ev;
}

really_inline void value_to(const object& jv, const char* key, json::object& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    v = ev;
}

really_inline void value_to(const object& jv, const char* key, json::array& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    v = ev;
}

really_inline void value_to(const object& jv, const char* key, std::string& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    json::value_to(ev, v);
}

really_inline void value_to(const object& jv, const char* key,
                            std::string_view& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    json::value_to(ev, v);
}

really_inline void value_to(const object& jv, const char* key,
                            binance::error& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    v = ev;
}

really_inline void value_to(const object& jv, const char* key, int64_t& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    json::value_to(ev, v);
}

template<typename ChronoScale = std::chrono::milliseconds>
really_inline void value_to(const object& jv, const char* key, time_point_t& v)
{
  auto [ev, e] = jv[key];
  if (!e)
  {
    json::value_to<ChronoScale>(ev, v);
  }
}

really_inline void value_to(const object& jv, const char* key, int& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    json::value_to(ev, v);
}

really_inline void value_to(const object& jv, const char* key, double& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    json::value_to(ev, v);
}

really_inline void value_to(const object& jv, const char* key, bool& v)
{
  auto [ev, e] = jv[key];
  if (!e)
    v = ev;
}

template<class T, class = std::enable_if_t<std::is_assignable_v<T, object&>>>
really_inline void value_to(const object& jb, const char* key, T& v)
{
  v = jb[key];
}

template<class T>
really_inline void value_to(const json::object& jb, const char* key,
                            std::vector<T>& v)
{
  const json::array& jr = jb[key];

  v.resize(jr.size());
  auto it = v.begin();

  for (const auto& e : jr)
  {
    (*it) = e;
    ++it;
  }
}

template<class T>
really_inline void value_to(const json::array& jr, std::vector<T>& v)
{
  v.resize(jr.size());
  auto it = v.begin();

  for (const auto& e : jr)
  {
    (*it) = e;
    ++it;
  }
}

template<class ChronoScale = std::chrono::milliseconds, class T, class... Args>
really_inline void value_to(json::array::iterator& it,
                            const json::array::iterator& end, T& v, Args&... vs)
{
  if constexpr (std::is_same_v<T, time_point_t>)
    json::value_to<ChronoScale>(*it, v);
  else
    json::value_to(*it, v);
  if constexpr (sizeof...(vs) > 0)
  {
    if (++it != end)
      value_to<ChronoScale>(it, end, std::forward<Args&>(vs)...);
  }
}

// TODO: Does this collision with value_to(json::array&, std::vector<T> ...) ?
template<class ChronoScale = std::chrono::milliseconds, class... Args>
really_inline void value_to(const json::array& jr, Args&... vs)
{
  auto it = jr.begin();
  value_to<ChronoScale>(it, jr.end(), std::forward<Args&>(vs)...);
}
}  // namespace json
}  // namespace binance

#endif
