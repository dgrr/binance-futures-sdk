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
// https://binance-docs.github.io/apidocs/futures/en/#exchange-information
struct exchange_info : public query_args
{
  struct rate_limit
  {
    int interval_num;        // intervalNum
    int limit;               // limit
    string_type interval;    // interval
    string_type limit_type;  // rateLimitType
    rate_limit& operator=(const json::object& jb)
    {
      _value_to("intervalNum", interval_num);
      _value_to("limit", limit);
      _value_to("interval", interval);
      _value_to("rateLimitType", limit_type);
      return *this;
    }
  };

  struct symbol_data
  {
    int price_precision;      // pricePrecision
    int qty_precision;        // quantityPrecision
    int base_precision;       // baseAssetPrecision
    int quote_precision;      // quotePrecision
    string_type symbol;       // symbol
    string_type status;       // status
    string_type base_asset;   // baseAsset
    string_type quote_asset;  // quoteAsset
    double m_margin_pct;      // maintMarginPercent
    double r_margin_pct;      // requiredMarginPercent
    // TODO: underlyingType
    // TODO: underlyingSubType
    double settle_plan;      // settlePlan
    double trigger_protect;  // triggerProtect
    // TODO: filters
    // TODO: OrderType
    // TODO: timeInForce
    symbol_data& operator=(const json::object& jb)
    {
      _value_to("pricePrecision", price_precision);
      _value_to("quantityPrecision", qty_precision);
      _value_to("baseAssetPrecision", base_precision);
      _value_to("quotePrecision", quote_precision);
      _value_to("symbol", symbol);
      _value_to("status", status);
      _value_to("baseAsset", base_asset);
      _value_to("quoteAsset", quote_asset);
      _value_to("mainMarginPercent", m_margin_pct);
      _value_to("requiredMarginPercent", r_margin_pct);
      _value_to("settlePlan", settle_plan);
      _value_to("triggerProtect", trigger_protect);
      return *this;
    }
  };

  time_point_t server_time;             // serverTime
  std::vector<rate_limit> rate_limits;  // rateLimits
  std::vector<symbol_data> symbols;     // symbols

  exchange_info& operator=(const json::object& jb)
  {
    _value_to("serverTime", server_time);
    _value_to("rateLimits", rate_limits);
    _value_to("symbols", symbols);
    return *this;
  }
};
// TODO: Get it from websocket?? Or commonly declare?
struct price_point
{
  double price;
  double qty;
  price_point& operator=(const json::array& jr)
  {
    json::value_to(jr.begin(), jr.end(), price, qty);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#order-book
struct orderbook : public query_args
{
  int64_t last_update_id;         // lastUpdateId
  time_point_t output_time;       // E
  time_point_t x_time;            // T
  std::vector<price_point> bids;  // bids
  std::vector<price_point> asks;  // asks

  orderbook() = delete;
  orderbook(const std::string& symbol, size_t limit = 500)
      : query_args{{"symbol", symbol}, {"limit", limit}}
  {
  }

  orderbook& set_limit(size_t limit)
  {
    insert_kv({"limit", limit});
    return *this;
  }

  orderbook& operator=(const json::object& jb)
  {
    _value_to("lastUpdateId", last_update_id);
    _value_to("E", output_time);
    _value_to("T", x_time);
    _value_to("bids", bids);
    _value_to("asks", asks);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#recent-trades-list
struct recent_trades : public query_args
{
  struct trade
  {
    int64_t id;           // id
    time_point_t time;    // time
    bool is_buyer_maker;  // isBuyerMaker
    double price;         // price
    double qty;           // qty
    double quote_qty;     // quoteQty
    trade& operator=(const json::object& jb)
    {
      _value_to("id", id);
      _value_to("time", time);
      _value_to("isBuyerMaker", is_buyer_maker);
      _value_to("price", price);
      _value_to("qty", qty);
      _value_to("quoteQty", quote_qty);
      return *this;
    }
  };

  std::vector<trade> trades;

  recent_trades() = delete;
  recent_trades(const std::string& symbol)
      : query_args{{"symbol", symbol}}
  {
  }

  recent_trades& set_limit(size_t limit)
  {
    insert_kv({"limit", limit});
    return *this;
  }

  recent_trades& operator=(const json::array& jr)
  {
    json::value_to(jr, trades);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#old-trades-lookup-market_data
// https://binance-docs.github.io/apidocs/futures/en/#compressed-aggregate-trades-list
// https://binance-docs.github.io/apidocs/futures/en/#mark-price
struct mark_price : public query_args
{
  string_type symbol;              // symbol
  double price;                    // markPrice
  double index_price;              // indexPrice
  double last_funding_rate;        // lastFundingRate
  time_point_t next_funding_time;  // nextFundingTime
  time_point_t time;               // time

  mark_price() = delete;
  // TODO: Support vector of mark_price
  mark_price(const std::string& symbol)
      : query_args{{"symbol", symbol}}
  {
  }

  mark_price& operator=(const json::object& jb)
  {
    _value_to("symbol", symbol);
    _value_to("markPrice", price);
    _value_to("indexPrice", index_price);
    _value_to("lastFundingRate", last_funding_rate);
    _value_to("nextFundingTime", next_funding_time);
    _value_to("time", time);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#get-funding-rate-history
// https://binance-docs.github.io/apidocs/futures/en/#24hr-ticker-price-change-statistics
struct price_ticker : public query_args
{
  string_type symbol;  // symbol
  double price;        // price
  time_point_t time;   // time

  price_ticker() = delete;
  // TODO: Support multiple
  price_ticker(const std::string& symbol)
      : query_args{{"symbol", symbol}}
  {
  }

  price_ticker& operator=(const json::object& jb)
  {
    _value_to("symbol", symbol);
    _value_to("price", price);
    _value_to("time", time);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#symbol-order-book-ticker
// https://binance-docs.github.io/apidocs/futures/en/#get-all-liquidation-orders
// https://binance-docs.github.io/apidocs/futures/en/#open-interest
// https://binance-docs.github.io/apidocs/futures/en/#open-interest-statistics
// https://binance-docs.github.io/apidocs/futures/en/#top-trader-long-short-ratio-accounts-market_data
// https://binance-docs.github.io/apidocs/futures/en/#top-trader-long-short-ratio-positions
// https://binance-docs.github.io/apidocs/futures/en/#long-short-ratio
// https://binance-docs.github.io/apidocs/futures/en/#taker-buy-sell-volume
// https://binance-docs.github.io/apidocs/futures/en/#historical-blvt-nav-kline-candlestick
// https://binance-docs.github.io/apidocs/futures/en/#get-current-position-mode-user_data
struct get_position_mode : public query_args
{
  bool dual_position;

  get_position_mode()        = default;
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

  kline_data() = delete;
  // TODO: Interval to ENUM
  kline_data(const std::string& symbol, const std::string& interval)
      : query_args{{"symbol", symbol}, {"interval", interval}}
  {
  }
  setter(kline_data&, set_start_time, int64_t, "startTime", start);
  setter(kline_data&, set_end_time, int64_t, "endTime", end);
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
  string_type client_oid;     // clientOrderId
  string_type type;           // type
  string_type side;           // side
  string_type pos_side;       // positionSide
  string_type status;         // status
  string_type symbol;         // symbol
  string_type time_in_force;  // timeInForce
  string_type working_type;   // workingType
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

  place_order() = delete;
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

  cancel_order() = delete;
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
// https://binance-docs.github.io/apidocs/futures/en/#cancel-all-open-orders-trade
struct cancel_order_all : public query_args
{
  int code;
  string_type msg;

  cancel_order_all() = delete;
  cancel_order_all(const std::string& symbol)
      : query_args{{"symbol", symbol}}
  {
  }

  setter(cancel_order_all&, set_recv_window, int64_t, "recvWindow", win);

  cancel_order_all& operator=(const json::object& jb)
  {
    _value_to("code", code);
    _value_to("msg", msg);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#query-current-open-order-user_data
struct current_open_order : public query_args
{
  order_base result;

  current_open_order() = delete;
  current_open_order(const std::string& symbol)
      : query_args{{"symbol", symbol}}
  {
  }
  setter(current_open_order&, set_order_id, int64_t, "orderId", order_id);
  setter(current_open_order&, set_client_order_id, const std::string&,
         "origClientOrderId", id);
  setter(current_open_order&, set_recv_window, int64_t, "recvWindow", win);

  current_open_order& operator=(const json::object& jb)
  {
    result = jb;
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#current-all-open-orders-user_data
struct current_open_order_all : public query_args
{
  std::vector<order_base> orders;

  current_open_order_all() = default;
  setter(current_open_order_all&, set_symbol, const std::string&, "symbol",
         symbol);
  setter(current_open_order_all&, set_recv_window, int64_t, "recvWindow", win);

  current_open_order_all& operator=(const json::array& jr)
  {
    json::value_to(jr, orders);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#start-user-data-stream-user_stream
struct listen_key : public query_args
{
  string_type key;

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
