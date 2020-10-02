#ifndef BINANCE_WEBSOCKET_MESSAGES_HPP
#define BINANCE_WEBSOCKET_MESSAGES_HPP

#include <binance/json.hpp>

namespace binance
{
namespace websocket
{
namespace messages
{
struct head
{
  std::string_view type;
  time_point_t event_time;
  std::string_view symbol;

  head()        = default;
  head& operator=(const json::object& jb)
  {
    json::value_to(jb, "e", type);
    json::value_to(jb, "E", event_time);
    json::value_to(jb, "s", symbol);
    return *this;
  }
};
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
    json::value_to(jb, "e", event_type);
    json::value_to(jb, "s", symbol);
    json::value_to(jb, "E", event_time);
    json::value_to(jb, "T", next_fund_time);
    json::value_to(jb, "p", price);
    json::value_to(jb, "i", index_price);
    json::value_to(jb, "r", funding_rate);
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
    json::value_to(jb, "s", symbol);
    json::value_to(jb, "t", start_time);
    json::value_to(jb, "T", close_time);
    json::value_to(jb, "i", interval);
    json::value_to(jb, "f", first_trade_id);
    json::value_to(jb, "L", last_trade_id);
    json::value_to(jb, "o", open_price);
    json::value_to(jb, "c", close_price);
    json::value_to(jb, "h", high_price);
    json::value_to(jb, "l", low_price);
    json::value_to(jb, "v", base_volume);
    json::value_to(jb, "n", trades);
    json::value_to(jb, "x", closed);
    json::value_to(jb, "q", quote_volume);
    json::value_to(jb, "n", trades);
    json::value_to(jb, "V", taker_base_buy_vol);
    json::value_to(jb, "Q", taker_quote_buy_vol);
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
    json::value_to(jb, "e", event_type);
    json::value_to(jb, "s", symbol);
    json::value_to(jb, "E", event_time);
    json::value_to(jb, "c", close_price);
    json::value_to(jb, "o", open_price);
    json::value_to(jb, "h", high_price);
    json::value_to(jb, "l", low_price);
    json::value_to(jb, "v", base_vol);
    json::value_to(jb, "q", quote_vol);
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
    json::value_to(jb, "e", event_type);
    json::value_to(jb, "E", event_time);
    json::value_to(jb, "s", symbol);
    json::value_to(jb, "p", price_change);
    json::value_to(jb, "P", price_change_pct);
    json::value_to(jb, "w", w_avg_price);
    json::value_to(jb, "c", last_price);
    json::value_to(jb, "Q", last_qty);
    json::value_to(jb, "o", open_price);
    json::value_to(jb, "h", high_price);
    json::value_to(jb, "l", low_price);
    json::value_to(jb, "v", base_vol);
    json::value_to(jb, "q", quote_vol);
    json::value_to(jb, "F", first_trade_id);
    json::value_to(jb, "L", last_trade_id);
    json::value_to(jb, "n", trades);
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
    json::value_to(jb, "u", order_book_id);
    json::value_to(jb, "T", transaction_time);
    json::value_to(jb, "E", event_time);
    json::value_to(jb, "s", symbol);
    json::value_to(jb, "b", best_bid_price);
    json::value_to(jb, "B", best_bid_qty);
    json::value_to(jb, "a", best_ask_price);
    json::value_to(jb, "A", best_ask_qty);
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
    json::value_to(jb, "T", trade_time);
    json::value_to(jb, "s", symbol);
    json::value_to(jb, "S", side);
    json::value_to(jb, "o", order_type);
    json::value_to(jb, "X", order_status);
    json::value_to(jb, "f", tif);
    json::value_to(jb, "q", qty);
    json::value_to(jb, "ap", avg_price);
    json::value_to(jb, "l", last_filled_qty);
    json::value_to(jb, "z", acc_filled);
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
// https://binance-docs.github.io/apidocs/futures/en/#partial-book-depth-streams
// https://binance-docs.github.io/apidocs/futures/en/#diff-book-depth-streams
// https://binance-docs.github.io/apidocs/futures/en/#blvt-info-streams
// https://binance-docs.github.io/apidocs/futures/en/#blvt-nav-kline-candlestick-streams
}  // namespace messages
}  // namespace websocket
}  // namespace binance

#endif
