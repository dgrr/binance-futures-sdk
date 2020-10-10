#ifndef BINANCE_WEBSOCKET_MESSAGES_HPP
#define BINANCE_WEBSOCKET_MESSAGES_HPP

#include <binance/json.hpp>

namespace binance
{
namespace websocket
{
namespace messages
{
#define _value_to(x, e) json::value_to(jb, x, e)
// https://binance-docs.github.io/apidocs/futures/en/#mark-price-stream
struct mark_price
{
  std::string_view event_type;  // e
  std::string_view symbol;      // s
  time_point_t event_time;      // E
  time_point_t next_fund_time;  // T
  double price;                 // p
  double index_price;           // i
  double funding_rate;          // r

  mark_price& operator=(const json::object& jb)
  {
    _value_to("e", event_type);
    _value_to("s", symbol);
    _value_to("E", event_time);
    _value_to("T", next_fund_time);
    _value_to("p", price);
    _value_to("i", index_price);
    _value_to("r", funding_rate);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#mark-price-stream-for-all-market
struct mark_price_all
{
  std::vector<mark_price> marks;
  mark_price_all& operator=(const json::array& jb)
  {
    json::value_to(jb, marks);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#kline-candlestick-data
struct kline
{
  time_point_t start_time;     // t
  time_point_t close_time;     // T
  std::string_view symbol;     // s
  std::string_view interval;   // i
  int64_t first_trade_id;      // f
  int64_t last_trade_id;       // L
  double open_price;           // o
  double close_price;          // c
  double high_price;           // h
  double low_price;            // l
  double base_volume;          // v
  int64_t trades;              // n
  bool closed;                 // x
  double quote_volume;         // q
  double taker_base_buy_vol;   // V
  double taker_quote_buy_vol;  // Q

  kline& operator=(const json::object& jb)
  {
    _value_to("s", symbol);
    _value_to("t", start_time);
    _value_to("T", close_time);
    _value_to("i", interval);
    _value_to("f", first_trade_id);
    _value_to("L", last_trade_id);
    _value_to("o", open_price);
    _value_to("c", close_price);
    _value_to("h", high_price);
    _value_to("l", low_price);
    _value_to("v", base_volume);
    _value_to("n", trades);
    _value_to("x", closed);
    _value_to("q", quote_volume);
    _value_to("n", trades);
    _value_to("V", taker_base_buy_vol);
    _value_to("Q", taker_quote_buy_vol);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#individual-symbol-mini-ticker-stream
struct mini_ticker
{
  std::string_view event_type;  // e
  std::string_view symbol;      // s
  time_point_t event_time;      // E
  double close_price;           // c
  double open_price;            // o
  double high_price;            // h
  double low_price;             // l
  double base_vol;              // v
  double quote_vol;             // q
  mini_ticker& operator=(const json::object& jb)
  {
    _value_to("e", event_type);
    _value_to("s", symbol);
    _value_to("E", event_time);
    _value_to("c", close_price);
    _value_to("o", open_price);
    _value_to("h", high_price);
    _value_to("l", low_price);
    _value_to("v", base_vol);
    _value_to("q", quote_vol);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#all-market-mini-tickers-stream
struct mini_ticker_all
{
  std::vector<mark_price> tickers;
  mini_ticker_all& operator=(const json::array& jb)
  {
    json::value_to(jb, tickers);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#individual-symbol-ticker-streams
struct ticker
{
  std::string_view event_type;  // e
  time_point_t event_time;      // E
  std::string_view symbol;      // s
  double price_change;          // p
  double price_change_pct;      // P
  double w_avg_price;           // w
  double last_price;            // c
  double last_qty;              // Q
  double open_price;            // o
  double high_price;            // h
  double low_price;             // l
  double base_vol;              // v
  double quote_vol;             // q
  // TODO: O, C
  int64_t first_trade_id;  // F
  int64_t last_trade_id;   // L
  int64_t trades;          // n
  ticker& operator=(const json::object& jb)
  {
    _value_to("e", event_type);
    _value_to("E", event_time);
    _value_to("s", symbol);
    _value_to("p", price_change);
    _value_to("P", price_change_pct);
    _value_to("w", w_avg_price);
    _value_to("c", last_price);
    _value_to("Q", last_qty);
    _value_to("o", open_price);
    _value_to("h", high_price);
    _value_to("l", low_price);
    _value_to("v", base_vol);
    _value_to("q", quote_vol);
    _value_to("F", first_trade_id);
    _value_to("L", last_trade_id);
    _value_to("n", trades);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#all-market-tickers-streams
struct ticker_all
{
  std::vector<ticker> tickers;
  ticker_all& operator=(const json::array& jb)
  {
    json::value_to(jb, tickers);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#individual-symbol-book-ticker-streams
struct book_ticker
{
  int64_t order_book_id;     // u
  int64_t transaction_time;  // T
  time_point_t event_time;   // E
  std::string_view symbol;   // s
  double best_bid_price;     // b
  double best_bid_qty;       // B
  double best_ask_price;     // a
  double best_ask_qty;       // A
  book_ticker& operator=(const json::object& jb)
  {
    _value_to("u", order_book_id);
    _value_to("T", transaction_time);
    _value_to("E", event_time);
    _value_to("s", symbol);
    _value_to("b", best_bid_price);
    _value_to("B", best_bid_qty);
    _value_to("a", best_ask_price);
    _value_to("A", best_ask_qty);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#all-book-tickers-stream
struct book_ticker_all
{
  std::vector<book_ticker> book_tickers;
  book_ticker_all& operator=(const json::array& jb)
  {
    json::value_to(jb, book_tickers);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#liquidation-order-streams
struct liq_order
{
  time_point_t trade_time;        // T
  std::string_view symbol;        // s
  std::string_view side;          // S
  std::string_view order_type;    // o
  std::string_view order_status;  // X
  std::string_view tif;           // f
  double qty;                     // q
  double avg_price;               // ap
  double last_filled_qty;         // l
  double acc_filled;              // z
  liq_order& operator=(const json::object& jb)
  {
    _value_to("T", trade_time);
    _value_to("s", symbol);
    _value_to("S", side);
    _value_to("o", order_type);
    _value_to("X", order_status);
    _value_to("f", tif);
    _value_to("q", qty);
    _value_to("ap", avg_price);
    _value_to("l", last_filled_qty);
    _value_to("z", acc_filled);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#all-market-liquidation-order-streams
struct liq_order_all
{
  std::vector<liq_order> orders;
  liq_order_all& operator=(const json::array& jv)
  {
    json::value_to(jv, orders, "o");
    return *this;
  }
};
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
// https://binance-docs.github.io/apidocs/futures/en/#partial-book-depth-streams
struct partial_book_depth
{
  std::string_view event_type;  // e
  std::string_view symbol;      // s
  time_point_t event_time;      // E
  time_point_t x_time;          // T
  // TODO: What is U and u and pu
  std::vector<price_point> bids;  // b
  std::vector<price_point> asks;  // a
  partial_book_depth& operator=(const json::object& jb)
  {
    _value_to("e", event_type);
    _value_to("s", symbol);
    _value_to("E", event_time);
    _value_to("T", x_time);
    _value_to("b", bids);
    _value_to("a", asks);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#diff-book-depth-streams
struct book_depth
{
  std::string_view event_type;    // e
  std::string_view symbol;        // s
  time_point_t event_time;        // E
  time_point_t x_time;            // T
  int64_t first_id;               // U
  int64_t final_id;               // u
  int64_t last_final_id;          // pu
  std::vector<price_point> bids;  // b
  std::vector<price_point> asks;  // a

  book_depth& operator=(const json::object& jb)
  {
    _value_to("e", event_type);
    _value_to("s", symbol);
    _value_to("E", event_time);
    _value_to("T", x_time);
    _value_to("U", first_id);
    _value_to("u", final_id);
    _value_to("pu", last_final_id);
    _value_to("b", bids);
    _value_to("a", asks);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#blvt-info-streams
struct blvt_info
{
  struct basket
  {
    std::string_view symbol;  // s
    double position;          // n
    basket& operator=(const json::object& jb)
    {
      _value_to("s", symbol);
      _value_to("n", position);
      return *this;
    }
  };

  time_point_t event_time;      // E
  std::string_view event_type;  // e
  std::string_view blvt_name;   // s
  int target_leverage;          // t
  double token;                 // m
  double nav;                   // n
  double leverage;              // l
  double funding_ratio;         // f
  std::vector<basket> baskets;  // b

  blvt_info& operator=(const json::object& jb)
  {
    _value_to("e", event_type);
    _value_to("E", event_time);
    _value_to("s", blvt_name);
    _value_to("m", token);
    _value_to("b", baskets);
    _value_to("n", nav);
    _value_to("l", leverage);
    _value_to("t", target_leverage);
    _value_to("f", funding_ratio);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#blvt-nav-kline-candlestick-streams
using blvt_nav_kline = kline;
// https://binance-docs.github.io/apidocs/futures/en/#event-user-data-stream-expired
struct user_data_expired
{
  std::string_view event_type;  // e
  time_point_t event_time;      // E
  user_data_expired& operator=(const json::object& jb)
  {
    _value_to("e", event_type);
    _value_to("E", event_time);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#event-margin-call
struct user_margin_call
{
  struct position
  {
    std::string_view symbol;       // s
    std::string_view pos_side;     // ps
    std::string_view margin_type;  // mt
    double pos_amount;             // pa
    // TODO: `iw` issolated wallet?
    double mark_price;  // mp
    double u_pnl;       // up
    double m_margin;    // mm
    position& operator=(const json::object& jb)
    {
      _value_to("s", symbol);
      _value_to("ps", pos_side);
      _value_to("mt", margin_type);
      _value_to("pa", pos_amount);
      _value_to("mp", mark_price);
      _value_to("up", u_pnl);
      _value_to("mm", m_margin);
      return *this;
    }
  };

  std::string_view event_type;       // e
  time_point_t event_time;           // E
  double cw_balance;                 // cw
  std::vector<position> pos_margin;  // p
  user_margin_call& operator=(const json::object& jb)
  {
    _value_to("e", event_type);
    _value_to("E", event_time);
    _value_to("cw", cw_balance);
    _value_to("p", pos_margin);
    return *this;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#event-balance-and-position-update
// https://binance-docs.github.io/apidocs/futures/en/#event-order-update
struct user_order_update
{
  time_point_t order_time;             // T
  int64_t order_id;                    // i
  int64_t trade_id;                    // t
  std::string_view symbol;             // s
  std::string_view client_oid;         // c
  std::string_view side;               // S
  std::string_view order_type;         // o
  std::string_view time_in_force;      // f
  std::string_view exec_type;          // x
  std::string_view status;             // X
  std::string_view commission_asset;   // N
  std::string_view stop_working_type;  // wt
  std::string_view orig_order_type;    // ot
  std::string_view pos_side;           // ps
  double orig_qty;                     // q
  double orig_price;                   // p
  double avg_price;                    // ap
  double stop_price;                   // sp
  double last_filled_qty;              // l
  double acc_qty;                      // z
  double last_filled_price;            // L
  double comission;                    // n
  double bid_notional;                 // b
  double ask_notional;                 // a
  double closed_all;                   // cp
  double activation_price;             // ap
  double callback_rate;                // cr
  double realized_profit;              // rp
  bool is_maker;                       // m
  bool is_reduce_only;                 // R
};
#undef _value_to
}  // namespace messages
}  // namespace websocket
}  // namespace binance

#endif
