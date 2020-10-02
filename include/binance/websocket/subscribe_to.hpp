#ifndef BINANCE_WEBSOCKET_SUBSCRIBE_HPP
#define BINANCE_WEBSOCKET_SUBSCRIBE_HPP

#include <initializer_list>
#include <string>
#include <vector>

namespace binance
{
namespace websocket
{
namespace subscribe_to
{
class topic_path
{
protected:
  std::string topic_;

public:
  topic_path(const std::string& full_topic)
      : topic_(full_topic)
  {
  }
  topic_path(const std::string& symbol, const std::string& topic)
  {
    topic_ = symbol + topic;
  }
  const std::string& topic() const
  {
    return topic_;
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#aggregate-trade-streams
struct agg_trade : public topic_path
{
  agg_trade() = delete;
  agg_trade(const std::string& symbol)
      : topic_path(symbol, "@aggTrade")
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#mark-price-stream
struct mark_price : public topic_path
{
  mark_price() = delete;
  mark_price(const std::string& symbol, bool every_second = true)
      : topic_path(symbol,
                   "@markPrice" + std::string(every_second ? "@1s" : ""))
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#mark-price-stream-for-all-market
struct mark_price_all : public topic_path
{
  mark_price_all(bool every_second = true)
      : topic_path("!markPrice@arr" + std::string(every_second ? "@1s" : ""))
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#kline-candlestick-streams
struct kline : public topic_path
{
  kline() = delete;
  kline(const std::string& symbol, const std::string& interval)
      : topic_path(symbol, "@kline_" + interval)
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#individual-symbol-mini-ticker-stream
struct mini_ticker : public topic_path
{
  mini_ticker() = delete;
  mini_ticker(const std::string& symbol)
      : topic_path(symbol, "@miniTicker")
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#all-market-mini-tickers-stream
struct mini_ticker_all : public topic_path
{
  mini_ticker_all()
      : topic_path("!miniTicker@arr")
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#individual-symbol-ticker-streams
struct ticker : public topic_path
{
  ticker() = delete;
  ticker(const std::string& symbol)
      : topic_path(symbol, "@ticker")
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#all-market-tickers-streams
struct ticker_all : public topic_path
{
  ticker_all()
      : topic_path("!ticker@arr")
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#individual-symbol-book-ticker-streams
struct book_ticker : public topic_path
{
  book_ticker() = delete;
  book_ticker(const std::string& symbol)
      : topic_path(symbol, "@bookTicker")
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#all-book-tickers-stream
struct book_ticker_all : public topic_path
{
  book_ticker_all()
      : topic_path("!bookTicker")
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#liquidation-order-streams
struct liq_order : public topic_path
{
  liq_order() = delete;
  liq_order(const std::string& symbol)
      : topic_path(symbol, "@forceOrder")
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#all-market-liquidation-order-streams
struct liq_order_all : public topic_path
{
  liq_order_all()
      : topic_path("!forceOrder@arr")
  {
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#partial-book-depth-streams
struct partial_book_depth : public topic_path
{
  partial_book_depth() = delete;
  partial_book_depth(const std::string& symbol, std::string& speed)
      : topic_path(symbol, "@depth" + speed)
  {
  
  }
};
// https://binance-docs.github.io/apidocs/futures/en/#diff-book-depth-streams
// https://binance-docs.github.io/apidocs/futures/en/#blvt-info-streams
// https://binance-docs.github.io/apidocs/futures/en/#blvt-nav-kline-candlestick-streams
}  // namespace subscribe_to
}  // namespace websocket
}  // namespace binance

#endif
