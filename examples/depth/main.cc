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
  std::multimap<float_type, double> bids;  // ASC
  std::multimap<float_type, double,
                std::greater<float_type>> asks;  // DESC
  int precision;
  float_type convert_price(double p)
  {
    return float_type(p, precision);
  }
};

struct queued_message
{
  int64_t final_id;
  int64_t last_final_id;
  std::vector<std::pair<double, double>> bids;
  std::vector<std::pair<double, double>> asks;

  queued_message(int64_t last, int64_t id)
      : last_final_id(last)
      , final_id(id)
  {
  }
};

class WebSocket : public std::enable_shared_from_this<WebSocket>
{
  binance::websocket::stream& ws_;
  binance::http::stream& api_;
  binance::buffer buffer_;
  binance::json::parser parser_;

  std::vector<queued_message> queued_messages_;

  std::string symbol_;
  market_data data_;
  int64_t last_time_;
  int64_t final_id_;

public:
  WebSocket(binance::websocket::stream& ws, binance::http::stream& api,
            std::string s, int precision)
      : ws_(ws)
      , api_(api)
      , symbol_(s)
      , final_id_(0)
      , last_time_(0)
  {
    data_.precision = precision;
  }

  void start()
  {
    ws_.subscribe(binance::websocket::subscribe_to::book_depth(symbol_));
    get_book();
    read();
  }

private:
  void get_book()
  {
    final_id_ = 0;
    data_.asks.clear();
    data_.bids.clear();

    std::cout << "Getting orderbook" << std::endl;

    api_.async_read<binance::http::messages::orderbook>(
        std::bind(&WebSocket::on_orderbook_read, shared_from_this(),
                  std::placeholders::_1),
        symbol_, 1000);
  }

  void on_orderbook_read(binance::http::messages::orderbook* ob)
  {
    for (auto& ask : ob->asks)
    {
      auto v = data_.convert_price(ask.price);
      data_.asks.insert({v, ask.qty});
    }

    for (auto& bid : ob->bids)
    {
      auto v = data_.convert_price(bid.price);
      data_.bids.insert({v, bid.qty});
    }

    final_id_  = ob->last_update_id;
    last_time_ = ob->output_time.time_since_epoch().count() / 1000000;

    std::sort(queued_messages_.begin(), queued_messages_.end(),
              [](const auto& a, const auto& b) -> bool {
                return a.final_id < b.final_id;
              });

    for (auto& msg : queued_messages_)
    {
      if (msg.final_id < ob->last_update_id || final_id_ == msg.final_id)
        continue;

      for (auto& ask : msg.asks)
      {
        binance::websocket::messages::price_point pp;
        pp.price = ask.first;
        pp.qty   = ask.second;

        handle_book_event(data_.asks, pp);
      }

      for (auto& bid : msg.bids)
      {
        binance::websocket::messages::price_point pp;
        pp.price = bid.first;
        pp.qty   = bid.second;

        handle_book_event(data_.bids, pp);
      }

      final_id_ = msg.final_id;
    }

    queued_messages_.clear();
  }

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

      int64_t ts = bd.event_time.time_since_epoch().count() / 1000000;
      if (ts < last_time_)
        return;
      last_time_ = ts;

      handle_depth(&bd);
    }

    read();
  }

  void handle_depth(binance::websocket::messages::book_depth* bd)
  {
    if (final_id_ == 0)
    {
      queued_message qm(bd->last_final_id, bd->final_id);

      for (auto& ask : bd->asks)
      {
        qm.asks.emplace_back(ask.price, ask.qty);
      }

      for (auto& bid : bd->bids)
      {
        qm.bids.emplace_back(bid.price, bid.qty);
      }

      queued_messages_.push_back(qm);

      return;
    }

    if (bd->last_final_id != final_id_)
    {
      std::cout << "Wrong last_final_id" << std::endl;
      get_book();
      return;
    }
    final_id_ = bd->final_id;

    for (auto& v : bd->asks)
      handle_book_event(data_.asks, v);
    for (auto& v : bd->bids)
      handle_book_event(data_.bids, v);

    std::cout << "Spread: "
              << data_.asks.rbegin()->first - data_.bids.rbegin()->first << "\r"
              << std::flush;
  }

  template<class T>
  void handle_book_event(T& mm,
                         const binance::websocket::messages::price_point& v)
  {
    static_assert(
        std::is_same_v<typename T::key_type, float_type>,
        "handle_book_operation only supports a multimap with key_type "
        "= float_type");

    auto price = data_.convert_price(v.price);
    auto it    = mm.find(price);
    if (it == mm.end())
    {
      if (v.qty > 0)
        mm.insert({price, v.qty});
      return;
    }

    if (v.qty == 0)
      mm.erase(it);
    else
      it->second = v.qty;
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
        std::make_shared<WebSocket>(ws, api, symbol, precision)->start();
      });
#else
      ws.connect(v->key, [&](auto v, auto ec) {
        std::make_shared<WebSocket>(ws, api, symbol, precision)->start();
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
