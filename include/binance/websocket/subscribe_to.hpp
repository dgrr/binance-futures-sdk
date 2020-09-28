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

struct agg_trade : public topic_path
{
  agg_trade() = delete;
  agg_trade(const std::string& symbol)
      : topic_path(symbol, "@aggTrade")
  {
  }
};
struct mark_price : public topic_path
{
  mark_price() = delete;
  mark_price(const std::string& symbol, const std::string& ts = "@1s")
      : topic_path(symbol, "@markPrice" + ts)
  {
  }
};
struct kline : public topic_path
{
  kline() = delete;
  kline(const std::string& symbol, const std::string& interval)
      : topic_path(symbol, "@kline_" + interval)
  {
  }
};
}  // namespace subscribe_to
}  // namespace websocket
}  // namespace binance

#endif
