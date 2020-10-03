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
#define setter(R, VCb, VType, K, V) \
  R VCb(VType V)                    \
  {                                 \
    insert_kv({K, V});              \
    return *this;                   \
  }
#define _value_to(x, e) json::value_to(jb, x, e)
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
  setter(kline_data&, set_start_time, size_t, "startTime", start);
  setter(kline_data&, set_end_time, size_t, "endTime", end);
  setter(kline_data&, set_interval, const std::string&, "interval", interval);
  setter(kline_data&, set_limit, size_t, "limit", limit);

  kline_data& operator=(const json::array& jb)
  {
    json::value_to(jb, klines);
    return *this;
  }
};

struct order_base
{
  // TODO: cumQty, cumQuote, origQty, reduceOnly, closePosition, origType
  time_point_t update_time;   // updateTime
  int64_t order_id;           // orderId
  double executed_qty;        // executedQty
  double avg_price;           // avgPrice
  double price;               // price
  double price_rate;          // priceRate
  double stop_price;          // stopPrice
  double activate_price;      // activatePrice
  std::string client_oid;     // clientOrderId
  std::string type;           // type
  std::string side;           // side
  std::string pos_side;       // positionSide
  std::string status;         // status
  std::string symbol;         // symbol
  std::string time_in_force;  // timeInForce
  std::string working_type;   // workingType
  order_base& operator=(const json::object& jb)
  {
    _value_to("updateTime", update_time);
    _value_to("orderId", order_id);
    _value_to("executedQty", executed_qty);
    _value_to("avgPrice", avg_price);
    _value_to("price", price);
    _value_to("priceRate", price_rate);
    _value_to("stopPrice", stop_price);
    _value_to("activatePrice", activate_price);
    _value_to("clientOrderId", client_oid);
    _value_to("type", type);
    _value_to("side", side);
    _value_to("positionSide", pos_side);
    _value_to("status", status);
    _value_to("symbol", symbol);
    _value_to("timeInForce", time_in_force);
    _value_to("workingType", working_type);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#new-order-trade
struct place_order : public query_args
{
  order_base result;

  place_order(const std::string& symbol, order_side side, order_type type)
      : query_args{{"symbol", symbol},
                   {"side", order_side_string[side]},
                   {"type", order_type_string[type]}}
  {
  }

  setter(place_order&, set_qty, double, "quantity", qty);
  setter(place_order&, set_price, double, "price", price);
  setter(place_order&, set_reduce_only, const std::string&, "reduceOnly",
         reduce_only);
  setter(place_order&, set_client_order_id, const std::string&,
         "newClientOrderId", order_id);
  setter(place_order&, set_stop_price, double, "stopPrice", price);
  setter(place_order&, set_close_position, const std::string&, "closePosition",
         close_p);
  setter(place_order&, set_activation_price, double, "activationPrice", price);
  setter(place_order&, set_callback_rate, double, "callbackRate", rate);
  setter(place_order&, set_recv_window, double, "recvWindow", recv_w);

  // setter(place_order&, set_symbol, const std::string&, "symbol", symbol)
  // setter(place_order&, set_side, const std::string&, "side", side)
  // setter(place_order&, set_type, const std::string&, "type", type)
  place_order& set_position_side(position_side ps)
  {
    insert_kv({"positionSide", position_side_string[ps]});
    return *this;
  }
  place_order& set_time_in_force(time_in_force tif)
  {
    insert_kv({"timeInForce", time_in_force_string[tif]});
    return *this;
  }
  place_order& set_working_type(working_type w_type)
  {
    insert_kv({"workingType", working_type_string[w_type]});
    return *this;
  }
  place_order& set_response_type(response_type r_type)
  {
    insert_kv({"newOrderRespType", response_type_string[r_type]});
    return *this;
  }

  place_order& operator=(const json::object& jb)
  {
    result = jb;
    return *this;
  }
};

// https://binance-docs.github.io/apidocs/futures/en/#cancel-order-trade
struct cancel_order : public query_args
{
  order_base result;

  cancel_order(const std::string& symbol, int64_t order_id)
      : query_args{{"symbol", symbol}, {"orderId", order_id}}
  {
  }
  cancel_order(const std::string& symbol, const std::string& order_oid)
      : query_args{{"symbol", symbol}, {"origClientOrderId", order_oid}}
  {
  }

  setter(cancel_order&, set_order_id, int64_t, "orderId", id);
  setter(cancel_order&, set_client_order_id, const std::string&,
         "origClientOrderId", id);
  setter(cancel_order&, set_recv_window, int64_t, "recvWindow", recv_window);

  cancel_order& operator=(const json::object& jb)
  {
    result = jb;
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
#undef _value_to
#undef setter
}  // namespace messages
}  // namespace http
}  // namespace binance

#endif
