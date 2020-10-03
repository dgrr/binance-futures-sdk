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
// https://binance-docs.github.io/apidocs/futures/en/#kline-candlestick-data
struct kline_data : public query_args
{
  struct kline
  {
    time_point_t open_time;
    double open;
    double high;
    double low;
    double close;
    double volume;
    time_point_t close_time;
    double quote_volume;
    int64_t trades;
    double taker_base_vol;
    double taker_quote_vol;
    kline& operator=(const json::array& jr)
    {
      json::value_to(jr.begin(), jr.end(), open_time, open, high, low, close,
                     volume, close_time, quote_volume, trades, taker_base_vol,
                     taker_quote_vol);
      return *this;
    }
  };

  std::vector<kline> klines;

  // TODO: Interval to ENUM
  kline_data(const std::string& symbol, const std::string& interval)
      : query_args{{"symbol", symbol}, {"interval", interval}}
  {
  }
  kline_data& set_start_time(size_t start)
  {
    insert_kv({"startTime", start});
    return *this;
  }
  kline_data& set_end_time(size_t end)
  {
    insert_kv({"endTime", end});
    return *this;
  }
  kline_data& set_interval(const std::string& interval)
  {
    insert_kv({"interval", interval});
    return *this;
  }
  kline_data& set_limit(size_t limit)
  {
    insert_kv({"limit", limit});
    return *this;
  }
  kline_data& operator=(const json::array& jb)
  {
    json::value_to(jb, klines);
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
// https://binance-docs.github.io/apidocs/futures/en/#start-user-data-stream-user_stream
struct listen_key : public query_args
{
  std::string key;

  listen_key()        = default;
  listen_key& operator=(const json::object& jb)
  {
    json::value_to(jb, "listenKey", key);
    return *this;
  }
};
struct empty_args : public query_args
{
  empty_args()        = default;
  empty_args& operator=(const json::value& v)
  {
    boost::ignore_unused(v);
    return *this;
  }
};
}  // namespace messages
}  // namespace http
}  // namespace binance

#endif
