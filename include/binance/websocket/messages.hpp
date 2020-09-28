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
// https://binance-docs.github.io/apidocs/futures/en/#kline-candlestick-data
struct kline
{
  time_point_t start_time;
  time_point_t close_time;
  std::string_view symbol;
  std::string_view interval;
  int64_t first_trade_id;
  int64_t last_trade_id;
  double open_price;
  double close_price;
  double high_price;
  double low_price;
  double base_volume;
  int64_t trades;
  bool closed;
  double quote_volume;
  double taker_base_buy_vol;
  double taker_quote_buy_vol;

  kline()
  {
  }
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
}  // namespace messages
}  // namespace websocket
}  // namespace binance

#endif
