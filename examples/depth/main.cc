#include <binance.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <config.hpp>
#include <iostream>

void parse_args(int argc, char* argv[],
                boost::program_options::options_description& desc,
                boost::program_options::variables_map& vm)
{
  namespace opt = boost::program_options;

  desc.add_options()("help,h", "Help message")(
      "symbol,s", opt::value<std::string>()->default_value("btcusdt"),
      "Symbol to be subscribed to")(
      "config,c", opt::value<std::string>()->default_value("binance.conf"),
      "Binance configuration file")(
      "url,U", opt::value<std::string>()->default_value(BINANCE_DEFAULT_URL),
      "Binance API base URL");

  opt::store(opt::parse_command_line(argc, argv, desc), vm);
  opt::notify(vm);
}

struct price_point
{
  double price;
  double size;
  price_point(double p, double s)
      : price(p)
      , size(s)
  {
  }
};

// overload operators to compare string and string_view on multimaps
bool operator<(const std::string& s, const std::string_view& sv)
{
  return s < sv;
}

class float_type
{
  int64_t n_;
  int precision_;

public:
  float_type() = delete;
  float_type(const float_type& other)
      : n_(other.n_)
      , precision_(other.precision_)
  {
  }
  float_type(double v, int precision)
  {
    n_         = int64_t(v * std::pow(10, precision));
    precision_ = precision;
  }
  operator double() const
  {
    return double(n_) / std::pow(10, precision_);
  }
  inline bool operator==(const float_type& other) const
  {
    return n_ == other.n_;
  }
  inline bool operator<(const float_type& other) const
  {
    return n_ < other.n_;
  }
  inline bool operator>(const float_type& other) const
  {
    return n_ > other.n_;
  }
  friend std::ostream& operator<<(std::ostream& os, const float_type& f);
};

std::ostream& operator<<(std::ostream& os, const float_type& f)
{
  os << static_cast<double>(f);
  return os;
}

struct market_data
{
  std::multimap<float_type, std::shared_ptr<price_point>> bids;  // ASC
  std::multimap<float_type, std::shared_ptr<price_point>,
                std::greater<float_type>>
      asks;  // DESC
  int precision;
  float_type convert_price(double p)
  {
    return float_type(p, precision);
  }
};

class WebSocket : public std::enable_shared_from_this<WebSocket>
{
  binance::websocket::stream& ws_;
  binance::http::stream& api_;
  binance::buffer buffer_;
  binance::json::parser parser_;
  std::shared_ptr<market_data> data_;

public:
  WebSocket(binance::websocket::stream& ws, binance::http::stream& api,
            std::shared_ptr<market_data> data)
      : ws_(ws)
      , api_(api)
      , data_(data)
  {
  }

  void start()
  {
    read();
  }

private:
  void read()
  {
    buffer_.clear();

    using namespace std::placeholders;
    ws_.async_read(buffer_,
                   std::bind(&WebSocket::on_read, shared_from_this(), _1));
  }

  void on_read(boost::system::error_code ec)
  {
    if (ec)
      throw ec;

    std::string_view e;
    const binance::json::object& jb = parser_.parse(buffer_).root();
    if (jb["e"].get(e) == simdjson::SUCCESS && e == "depthUpdate")
    {
      binance::websocket::messages::book_depth bd;

      bd = jb;
      for (auto& v : bd.bids)
      {
        float_type price = data_->convert_price(v.price);

        auto it = data_->bids.find(price);
        if (it == data_->bids.end())
        {
          if (v.qty > 0)
            data_->bids.insert(
                {price, std::make_shared<price_point>(v.price, v.qty)});
          continue;
        }

        if (v.qty == 0)
        {
          data_->bids.erase(it);
        }
        else
        {
          it->second->size = v.qty;
        }
      }

      for (auto& v : bd.asks)
      {
        float_type price = data_->convert_price(v.price);

        auto it = data_->asks.find(price);
        if (it == data_->asks.end())
        {
          if (v.qty > 0)
            data_->asks.insert(
                {price, std::make_shared<price_point>(v.price, v.qty)});
          continue;
        }

        if (v.qty == 0)
        {
          data_->asks.erase(it);
        }
        else
        {
          it->second->size = v.qty;
        }
      }

      std::cout << "Spread: "
                << data_->asks.rbegin()->second->price
                       - data_->bids.rbegin()->second->price
                << "\r" << std::flush;
    }

    read();
  }
};

class WebSocketSync : public std::enable_shared_from_this<WebSocketSync>
{
  binance::websocket::stream& ws_;
  binance::http::stream& api_;
  binance::buffer buffer_;
  binance::json::parser parser_;

  std::vector<std::string> queued_messages_;
  std::shared_ptr<market_data> data_;
  std::string symbol_;
  bool disabled_;

public:
  WebSocketSync(binance::websocket::stream& ws, binance::http::stream& api,
                std::string symbol, int precision)
      : ws_(ws)
      , api_(api)
      , symbol_(symbol)
      , disabled_(false)
  {
    data_            = std::make_shared<market_data>();
    data_->precision = precision;
    ws.subscribe(binance::websocket::subscribe_to::book_depth(symbol));
  }

  ~WebSocketSync()
  {
    std::make_shared<WebSocket>(ws_, api_, data_)->start();
  }

  void start()
  {
    read();
  }

private:
  void read()
  {
    buffer_.clear();

    if (disabled_)
      return;

    using namespace std::placeholders;
    ws_.async_read(buffer_,
                   std::bind(&WebSocketSync::on_read, shared_from_this(), _1));
  }

  void on_read(boost::system::error_code ec)
  {
    if (ec)
      throw ec;

    if (disabled_)  // event received after the stream has been disabled
    {
      const binance::json::object& jb = parser_.parse(buffer_).root();
      binance::websocket::messages::book_depth bd;

      bd = jb;
      for (auto& v : bd.bids)
      {
        float_type price = data_->convert_price(v.price);

        auto it = data_->bids.find(price);
        if (it == data_->bids.end())
        {
          if (v.qty > 0)
            data_->bids.insert(
                {price, std::make_shared<price_point>(v.price, v.qty)});
          continue;
        }

        if (v.qty == 0)
        {
          data_->bids.erase(it);
        }
        else
        {
          it->second->size = v.qty;
        }
      }

      for (auto& v : bd.asks)
      {
        float_type price = data_->convert_price(v.price);

        auto it = data_->asks.find(price);
        if (it == data_->asks.end())
        {
          if (v.qty > 0)
            data_->asks.insert(
                {price, std::make_shared<price_point>(v.price, v.qty)});
          continue;
        }

        if (v.qty == 0)
        {
          data_->asks.erase(it);
        }
        else
        {
          it->second->size = v.qty;
        }
      }

      return;
    }

    std::string msg = boost::beast::buffers_to_string(buffer_.data());
    queued_messages_.push_back(msg);

    if (queued_messages_.size() == 10)
    {
      using namespace std::placeholders;
      api_.async_read<binance::http::messages::orderbook>(
          std::bind(&WebSocketSync::on_orderbook_read, this, _1), symbol_,
          1000);
    }

    read();
  }

  void on_orderbook_read(binance::http::messages::orderbook* book)
  {
    std::string_view e;
    int64_t u;
    std::vector<std::string> msgs;

    // filter out queued_messages_
    for (const std::string& v : queued_messages_)
    {
      const binance::json::object& jb = parser_.parse(v).root();

      if (jb["e"].get(e) == simdjson::SUCCESS
          && jb["u"].get(u) == simdjson::SUCCESS && u > book->last_update_id)
      {
        // std::cout << book->last_update_id << " || " << U <<
        // std::endl;
        msgs.push_back(v);
      }
    }

    // insert the snapshot to the market data
    for (auto& bid : book->bids)
    {
      float_type d = data_->convert_price(bid.price);
      data_->bids.insert(
          {d, std::make_shared<price_point>(bid.price, bid.qty)});
    }

    for (auto& ask : book->asks)
    {
      float_type d = data_->convert_price(ask.price);
      data_->asks.insert(
          {d, std::make_shared<price_point>(ask.price, ask.qty)});
    }

    // process changes
    for (const std::string& v : msgs)
    {
      const binance::json::object& jb = parser_.parse(v).root();
      binance::websocket::messages::book_depth bd;
      std::cout << "Processing: " << v << std::endl;

      bd = jb;
      for (auto& v : bd.bids)
      {
        float_type price = data_->convert_price(v.price);

        auto it = data_->bids.find(price);
        if (it == data_->bids.end())
          continue;  // not found

        if (v.qty == 0)
        {
          data_->bids.erase(it);
        }
        else
        {
          it->second->size = v.qty;
        }
      }

      for (auto& v : bd.asks)
      {
        float_type price = data_->convert_price(v.price);

        auto it = data_->asks.find(price);
        if (it == data_->asks.end())
          continue;  // not found

        if (v.qty == 0)
        {
          data_->asks.erase(it);
        }
        else
        {
          it->second->size = v.qty;
        }
      }
    }

    disabled_ = true;
  }
};

int main(int argc, char* argv[])
{
  boost::program_options::variables_map args;
  boost::program_options::options_description desc(argv[0]);

  std::cout << "Using Binance version " << BINANCE_FUTURES_VERSION << std::endl;

  parse_args(argc, argv, desc, args);
  if (args.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }

  config cnf = config_parser::parse_file(args["config"].as<std::string>());

  binance::io_context ioc;
  binance::http::stream api(ioc, {cnf.api_key, cnf.api_secret},
                            args["url"].as<std::string>());
  binance::websocket::stream ws(ioc);

  api.async_connect();

  // handle signals and stop processing when SIGINT is received
  boost::asio::signal_set signals(ioc, SIGINT);
  signals.async_wait([&](const boost::system::error_code& ec, int n) {
    boost::ignore_unused(ec);
    boost::ignore_unused(n);
    ioc.stop();
  });

  std::string symbol = args["symbol"].as<std::string>();
  int precision      = 0;
  try
  {
    api.async_read<binance::http::messages::exchange_info>([&](auto* exi) {
      for (auto& v : exi->symbols)
      {
        if (boost::iequals(symbol, v.symbol))
        {
          precision = v.price_precision;
          break;
        }
      }
    });

    api.async_read<binance::http::messages::listen_key>([&](auto* v) {
      std::cout << "Listen key: " << v->key << std::endl;
      api.renew_listen_key();
#ifdef BINANCE_USE_STRING_VIEW
      ws.async_connect(std::string(v->key), [&](auto v, auto ec) {
        std::make_shared<WebSocketSync>(ws, api, symbol, precision)->start();
      });
#else
      ws.connect(v->key, [&](auto v, auto ec) {
        std::make_shared<WebSocketSync>(ws, api, symbol, precision)->start();
      });
#endif
    });

    ioc.run();
  }
  catch (const binance::error& ec)
  {
    std::cout << "error: " << ec << std::endl;
    exit(1);
  }
  catch (const boost::system::error_code& ec)
  {
    std::cout << "error: " << ec.message() << std::endl;
    exit(1);
  }

  return 0;
}
