#ifndef BINANCE_HTTP_MESSAGES_HPP
#define BINANCE_HTTP_MESSAGES_HPP

#include <binance/http/query_args.hpp>
#include <binance/json.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/url.hpp>
#include <chrono>
#include <string>

namespace binance
{
namespace http
{
void parse_args(std::string& target, std::string& query,
                const std::vector<query_args::key_value>& args)
{
  namespace http = boost::beast::http;
  for (auto& kv : args)
  {
    const auto& v = boost::variant2::visit(args_visitor_to_string(), kv.second);
    if (kv.first.empty())
      target += "/" + v;
    else
    {
      if (query.size() > 0)
        query += "&";
      query += kv.first + "=" + v;
    }
  }
}

template<class T>
struct paginator
{
  int current_page;
  int page_size;
  int total_num;
  int total_page;

  really_inline void set_current_page(size_t n)
  {
    T& v = static_cast<T&>(*this);
    v.insert_kv({"currentPage", std::to_string(n)});
  }
  really_inline void set_page_size(size_t n)
  {
    T& v = static_cast<T&>(*this);
    v.insert_kv({"pageSize", std::to_string(n)});
  }
  paginator& operator=(const json::object& jb)
  {
    json::value_to(jb, "totalNum", total_num);
    json::value_to(jb, "totalPage", total_page);
    json::value_to(jb, "currentPage", current_page);
    json::value_to(jb, "pageSize", page_size);
    return *this;
  }
};

namespace messages
{
// https://binance-docs.github.io/apidocs/futures/en/#get-current-position-mode-user_data
struct get_position_mode : public query_args
{
  bool dual_position;

  get_position_mode()
      : query_args{}
  {
  }
  get_position_mode& operator=(const json::object& jb)
  {
    json::value_to(jb, "dualSidePosition", dual_position);
    return *this;
  }
};
// https://docs.binance.com/futures/#place-an-order
struct place_limit_order : public query_args
{
  // order_id response
  std::string order_id;

  // place_limit_order() = delete;
  place_limit_order(const std::string& client_oid, const std::string& side,
                    const std::string& symbol, size_t leverage, double price,
                    size_t size)
      : query_args{{"clientOid", client_oid}, {"side", side},
                   {"symbol", symbol},        {"leverage", leverage},
                   {"price", price},          {"size", size},
                   {"type", "limit"}}
  {
  }
  place_limit_order& set_side(const std::string& side)
  {
    insert_kv({"side", side});
    return *this;
  }
  place_limit_order& set_symbol(const std::string& symbol)
  {
    insert_kv({"symbol", symbol});
    return *this;
  }
  place_limit_order& set_leverage(const size_t leverage)
  {
    insert_kv({"leverage", leverage});
    return *this;
  }
  place_limit_order& set_remark(const std::string& remark)
  {
    insert_kv({"remark", remark});
    return *this;
  }
  // requires set_stop_price and set_stop_price_type
  place_limit_order& set_stop(const std::string& stop)
  {
    insert_kv({"stop", stop});
    return *this;
  }
  place_limit_order& set_stop_price(const double price)
  {
    insert_kv({"stopPrice", price});
    return *this;
  }
  // TP, IP or MP
  place_limit_order& set_stop_price_type(const std::string& type)
  {
    insert_kv({"stopPriceType", type});
    return *this;
  }
  place_limit_order& set_price(const double price)
  {
    insert_kv({"price", price});
    return *this;
  }
  place_limit_order& set_size(const size_t size)
  {
    insert_kv({"size", size});
    return *this;
  }
  place_limit_order& set_reduce_only(bool reduce)
  {
    insert_kv({"reduceOnly", reduce});
    return *this;
  }
  place_limit_order& set_close_order(bool close)
  {
    insert_kv({"closeOrder", close});
    return *this;
  }
  place_limit_order& set_force_hold(bool hold)
  {
    insert_kv({"forceHold", hold});
    return *this;
  }
  place_limit_order& set_time_in_force(const std::string& timein)
  {
    insert_kv({"timeInForce", timein});
    return *this;
  }
  place_limit_order& set_post_only(bool post_only)
  {
    insert_kv({"postOnly", post_only});
    return *this;
  }
  place_limit_order& set_hidden(bool hidden)
  {
    insert_kv({"hidden", hidden});
    return *this;
  }
  place_limit_order& set_iceberg(bool iceberg)
  {
    insert_kv({"iceberg", iceberg});
    return *this;
  }
  place_limit_order& set_visible_size(size_t size)
  {
    insert_kv({"visibleSize", size});
    return *this;
  }

  place_limit_order& operator=(const json::object& jb)
  {
    json::value_to(jb, "orderId", order_id);
    return *this;
  }
};
// https://docs.binance.com/futures/#place-an-order
struct place_market_order : public query_args
{
  // order_id response
  std::string order_id;

  place_market_order(const std::string& client_oid, const std::string& side,
                     const std::string& symbol, size_t leverage, double price,
                     size_t size)
      : query_args{{"clientOid", client_oid}, {"side", side},
                   {"symbol", symbol},        {"leverage", leverage},
                   {"price", price},          {"size", size},
                   {"type", "market"}}
  {
  }
  place_market_order& set_side(const std::string& side)
  {
    insert_kv({"side", side});
    return *this;
  }
  place_market_order& set_symbol(const std::string& symbol)
  {
    insert_kv({"symbol", symbol});
    return *this;
  }
  place_market_order& set_leverage(const size_t leverage)
  {
    insert_kv({"leverage", leverage});
    return *this;
  }
  place_market_order& set_remark(const std::string& remark)
  {
    insert_kv({"remark", remark});
    return *this;
  }
  // requires set_stop_price and set_stop_price_type
  place_market_order& set_stop(const std::string& stop)
  {
    insert_kv({"stop", stop});
    return *this;
  }
  place_market_order& set_stop_price(const double price)
  {
    insert_kv({"stopPrice", price});
    return *this;
  }
  // TP, IP or MP
  place_market_order& set_stop_price_type(const std::string& type)
  {
    insert_kv({"stopPriceType", type});
    return *this;
  }
  place_market_order& set_price(const double price)
  {
    insert_kv({"price", price});
    return *this;
  }
  place_market_order& set_size(const size_t size)
  {
    insert_kv({"size", size});
    return *this;
  }
  place_market_order& set_reduce_only(bool reduce)
  {
    insert_kv({"reduceOnly", reduce});
    return *this;
  }
  place_market_order& set_close_order(bool close)
  {
    insert_kv({"closeOrder", close});
    return *this;
  }
  place_market_order& set_force_hold(bool hold)
  {
    insert_kv({"forceHold", hold});
    return *this;
  }

  place_market_order& operator=(const json::object& jb)
  {
    json::value_to(jb, "orderId", order_id);
    return *this;
  }
};
// https://docs.binance.com/#cancel-an-order
struct cancel_order : public query_args
{
  std::vector<std::string_view> canceled;

  cancel_order(const std::string& order_id)
      : query_args{{"", order_id}}
  {
  }

  cancel_order& operator=(const json::object& jb)
  {
    json::value_to(jb, "cancelledOrderIds", canceled);
    return *this;
  }
};
// TODO:
struct websocket_token
{
  struct server
  {
    int64_t ping_interval;
    std::string endpoint;
    std::string protocol;
    bool encrypt;
    int64_t ping_timeout;
    server& operator=(const json::object& jb)
    {
      json::value_to(jb, "pingInterval", ping_interval);
      json::value_to(jb, "endpoint", endpoint);
      json::value_to(jb, "protocol", protocol);
      json::value_to(jb, "encrypt", encrypt);
      json::value_to(jb, "pingTimeout", ping_timeout);
      return *this;
    }
  };
  std::vector<server> servers;
  std::string token;

  explicit websocket_token() = default;

  websocket_token& operator=(const json::object& jb)
  {
    json::value_to(jb, "token", token);
    json::value_to(jb, "instanceServers", servers);
    return *this;
  }
};
}  // namespace messages
}  // namespace http
}  // namespace binance

#endif
