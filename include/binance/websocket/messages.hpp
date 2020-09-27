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
  std::string_view topic;
  std::string_view subject;
  // avoid (huge) copies json::object data;
  head() = default;
  head(const json::object& jb)
  {
    (*this) = jb;
  }
  head& operator=(const json::object& jb)
  {
    json::value_to(jb, "type", type);
    json::value_to(jb, "topic", topic);
    json::value_to(jb, "subject", subject);
    return *this;
  }
};

// https://docs.binance.com/#symbol-ticker
// https://docs.binance.com/#all-symbols-ticker
struct ticker
{
  int64_t sequence;
  double best_ask;
  double size;
  double best_bid_size;
  double price;
  double best_ask_size;
  double best_bid;
  ticker(const json::object& jb)
  {
    (*this) = jb;
  }
  ticker& operator=(const json::object& jb)
  {
    json::value_to(jb, "sequence", sequence);
    json::value_to(jb, "bestAsk", best_ask);
    json::value_to(jb, "size", size);
    json::value_to(jb, "bestBidSize", best_bid_size);
    json::value_to(jb, "price", price);
    json::value_to(jb, "bestAskSize", best_ask_size);
    json::value_to(jb, "bestBid", best_bid);
    return *this;
  }
};  // namespace message

// https://docs.binance.com/#symbol-snapshot
// https://docs.binance.com/#market-snapshot
struct snapshot
{
  bool trading;
  std::string_view symbol;
  double buy;
  double sell;
  int64_t sort;
  double vol_value;
  std::string_view base_currency;
  std::string_view market;
  std::string_view quote_currency;
  std::string_view symbol_code;
  time_point_t time;
  double high;
  double vol;
  double low;
  double change_price;
  double change_rate;
  double last_traded_price;
  int64_t board;
  int64_t mark;
  snapshot(const json::object& jb)
  {
    (*this) = jb;
  }
  snapshot& operator=(const json::object& jb)
  {
    json::value_to(jb, "trading", trading);
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "buy", buy);
    json::value_to(jb, "sell", sell);
    json::value_to(jb, "sort", sort);
    json::value_to(jb, "volValue", vol_value);
    json::value_to(jb, "baseCurrency", base_currency);
    json::value_to(jb, "market", market);
    json::value_to(jb, "quoteCurrency", quote_currency);
    json::value_to(jb, "symbolCode", symbol_code);
    json::value_to(jb, "time", time);
    json::value_to(jb, "high", high);
    json::value_to(jb, "low", low);
    json::value_to(jb, "vol", vol);
    json::value_to(jb, "changePrice", change_price);
    json::value_to(jb, "changeRate", change_rate);
    json::value_to(jb, "lastTradedPrice", last_traded_price);
    json::value_to(jb, "board", board);
    json::value_to(jb, "mark", mark);
    return *this;
  }
};

// TODO: https://docs.binance.com/#level-2-market-data
struct level2 : public head
{
  struct level2_changes
  {
    struct market_change
    {
      double price;
      double size;
      int64_t sequence;
      market_change& operator=(const json::array& jr)
      {
        json::value_to(jr, price, size, sequence);
        return *this;
      }
    };
    std::vector<market_change> asks;
    std::vector<market_change> bids;
    level2_changes& operator=(const json::object& jb)
    {
      json::value_to(jb, "asks", asks);
      json::value_to(jb, "bids", bids);
      return *this;
    }
  };
  int64_t sequence_start;
  int64_t sequence_end;
  std::string_view symbol;
  level2_changes changes;

  level2() = default;
  level2(const json::object& jb)
  {
    (*this) = jb;
  }
  level2& operator=(const json::object& jb)
  {
    head::operator=(jb);
    const json::object& data = jb["data"];
    json::value_to(data, "sequenceStart", sequence_start);
    json::value_to(data, "sequenceEnd", sequence_end);
    json::value_to(data, "symbol", symbol);
    json::value_to(data, "changes", changes);
    // TODO: json::value_to(data, {{"sequenceStart", sequence_start},
    //                       {"sequenceEnd", sequence_end},
    //                       {"symbol", symbol},
    //                       {"changes", changes}});

    return *this;
  }
};

struct klines
{
  struct candle
  {
    time_point_t start_time;
    double open_price;
    double close_price;
    double high_price;
    double low_price;
    double volume;
    double amount;
    candle& operator=(const json::array& jb)
    {
      json::value_to(jb, start_time, open_price, close_price, high_price,
                     low_price, volume, amount);
      return *this;
    }
  };

  std::string_view symbol;
  time_point_t time;
  std::vector<candle> candles;
  klines()
  {
  }
  klines& operator=(const json::object& jb)
  {
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "time", time);
    json::value_to(jb, "candles", candles);
    return *this;
  }
};

struct match
{
  int64_t sequence;
  std::string_view symbol;
  std::string_view side;
  int size;
  double price;
  std::string_view taker_order_id;
  std::string_view maker_order_id;
  std::string_view taker_user_id;
  std::string_view trade_id;
  time_point_t time;

  match() = default;
  match(const json::object& jb)
  {
    (*this) = jb;
  }
  match& operator=(const json::object& jb)
  {
    json::value_to(jb, "sequence", sequence);
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "side", side);
    json::value_to(jb, "size", size);
    json::value_to(jb, "price", price);
    json::value_to(jb, "takerOrderId", taker_order_id);
    json::value_to(jb, "makerOrderId", maker_order_id);
    json::value_to(jb, "takerUserId", taker_user_id);
    json::value_to(jb, "tradeId", trade_id);
    json::value_to<std::chrono::nanoseconds>(jb, "ts", time);
    return *this;
  }
};

namespace level3
{
struct received
{
  int64_t sequence;
  std::string_view symbol;
  std::string_view order_id;
  time_point_t time;
  std::string_view client_oid;

  received(const json::object& jb)
  {
    (*this) = jb;
  }
  received& operator=(const json::object& jb)
  {
    json::value_to(jb, "sequence", sequence);
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "orderId", order_id);
    json::value_to<std::chrono::nanoseconds>(jb, "ts", time);
    json::value_to(jb, "clientOid", client_oid);
    return *this;
  }
};

struct open
{
  std::string_view symbol;
  int64_t sequence;
  std::string_view order_id;
  std::string_view side;
  double price;
  double size;
  time_point_t order_time;
  time_point_t time;

  open(const json::object& jb)
  {
    (*this) = jb;
  }
  open& operator=(const json::object& jb)
  {
    json::value_to(jb, "sequence", sequence);
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "side", side);
    json::value_to(jb, "size", size);
    json::value_to(jb, "orderId", order_id);
    json::value_to(jb, "price", price);
    json::value_to<std::chrono::nanoseconds>(jb, "ts", time);
    json::value_to<std::chrono::nanoseconds>(jb, "orderTime", order_time);
    return *this;
  }
};

struct done
{
  int64_t sequence;
  std::string_view symbol;
  std::string_view reason;
  std::string_view order_id;
  time_point_t time;

  done(const json::object& jb)
  {
    (*this) = jb;
  }
  done& operator=(const json::object& jb)
  {
    json::value_to(jb, "sequence", sequence);
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "reason", reason);
    json::value_to(jb, "orderId", order_id);
    json::value_to<std::chrono::nanoseconds>(jb, "ts", time);
    return *this;
  }
};

struct match
{
  int64_t sequence;
  std::string_view symbol;
  std::string_view side;
  double size;
  double price;
  double remain_size;
  std::string_view taker_order_id;
  std::string_view maker_order_id;
  std::string_view trade_id;
  time_point_t time;

  match() = default;
  match(const json::object& jb)
  {
    (*this) = jb;
  }
  match& operator=(const json::object& jb)
  {
    json::value_to(jb, "sequence", sequence);
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "side", side);
    json::value_to(jb, "size", size);
    json::value_to(jb, "price", price);
    json::value_to(jb, "remainSize", remain_size);
    json::value_to(jb, "takerOrderId", taker_order_id);
    json::value_to(jb, "makerOrderId", maker_order_id);
    json::value_to(jb, "tradeId", trade_id);
    json::value_to<std::chrono::nanoseconds>(jb, "ts", time);
    return *this;
  }
};

struct change
{
  int64_t sequence;
  std::string_view symbol;
  std::string_view order_id;
  time_point_t time;
  double size;
  change(const json::object& jb)
  {
    (*this) = jb;
  }
  change& operator=(const json::object& jb)
  {
    json::value_to(jb, "sequence", sequence);
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "orderId", order_id);
    json::value_to<std::chrono::nanoseconds>(jb, "ts", time);
    json::value_to(jb, "size", size);
    return *this;
  }
};
}  // namespace level3

struct index
{
  std::string_view symbol;
  int64_t granularity;
  time_point_t timestamp;
  double value;
  index()
  {
  }
  index& operator=(const json::object& jb)
  {
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "granularity", granularity);
    json::value_to(jb, "timestamp", timestamp);
    json::value_to(jb, "value", value);
    return *this;
  }
};

struct book_change
{
  int64_t sequence;
  std::string_view currency;
  double daily_int_rate;
  double annual_int_rate;
  int term;
  double size;
  std::string_view side;
  time_point_t ts;
  book_change()
  {
  }
  book_change& operator=(const json::object& jb)
  {
    json::value_to(jb, "sequence", sequence);
    json::value_to(jb, "currency", currency);
    json::value_to(jb, "dailyIntRate", daily_int_rate);
    json::value_to(jb, "annualIntRate", annual_int_rate);
    json::value_to(jb, "term", term);
    json::value_to(jb, "size", size);
    json::value_to(jb, "side", side);
    json::value_to(jb, "ts", ts);
    return *this;
  }
};

struct mark
{
  std::string_view symbol;
  int64_t granularity;
  time_point_t timestamp;
  double value;
  mark()
  {
  }
  mark& operator=(const json::object& jb)
  {
    json::value_to(jb, "symbol", symbol);
    json::value_to(jb, "granularity", granularity);
    json::value_to(jb, "timestamp", timestamp);
    json::value_to(jb, "value", value);
    return *this;
  }
};

// TODO: https://docs.binance.com/#private-channels
}  // namespace messages
}  // namespace websocket
}  // namespace binance

#endif