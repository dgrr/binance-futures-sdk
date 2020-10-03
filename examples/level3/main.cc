#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <iostream>
#include <binance.hpp>
#include <map>
#include <memory>

void parse_args(int argc, char* argv[],
                boost::program_options::options_description& desc,
                boost::program_options::variables_map& vm)
{
  namespace opt = boost::program_options;

  desc.add_options()("help,h", "Help message")(
      "key,k", opt::value<std::string>()->default_value(""), "Binance key")(
      "secret,s", opt::value<std::string>()->default_value(""),
      "Binance secret")("passphrase,p",
                       opt::value<std::string>()->default_value(""),
                       "Binance passphrase")(
      "url,U",
      opt::value<std::string>()->default_value("https://api.binance.com"),
      "Binance API base URL");

  opt::store(opt::parse_command_line(argc, argv, desc), vm);
  opt::notify(vm);
}

enum order_side
{
  NONE,
  BUY,
  SELL
};

struct order
{
  binance::time_point_t time;
  std::string id;
  order_side side;
  double price;
  double size;
  int64_t seq;

  order(const binance::websocket::messages::level3::received& op)
      : side(order_side::NONE)
  {
    time = op.time;
    id   = op.order_id;
    seq  = op.sequence;
  }
  order(const binance::websocket::messages::level3::open& op)
  {
    time  = op.time;
    id    = op.order_id;
    side  = op.side[0] == 'b' ? order_side::BUY : order_side::SELL;
    price = op.price;
    size  = op.size;
    seq   = op.sequence;
  }
  order(const binance::http::messages::orderbook_atomic::order& op, bool is_buy)
  {
    time  = op.time;
    id    = op.order_id;
    side  = is_buy ? order_side::BUY : order_side::SELL;
    price = op.price;
    size  = op.size;
  }
  order(const binance::websocket::messages::level3::done& op)
      : side(order_side::NONE)
  {
    time = op.time;
    id   = op.order_id;
    seq  = op.sequence;
  }
  inline bool operator==(const order& op)
  {
    return id == op.id;
  }
  inline void set_side(const binance::websocket::messages::level3::open& op)
  {
    side = op.side[0] == 'b' ? order_side::BUY : order_side::SELL;
  }
};

// overload operators to compare string and string_view on multimaps
bool operator<(const std::string& s, const std::string_view& sv)
{
  return s < sv;
}

struct market_data
{
  std::multimap<std::string, std::shared_ptr<order>, std::less<>> orders;
  std::multimap<double, std::shared_ptr<order>> bids;  // ASC
  std::multimap<double, std::shared_ptr<order>,
                std::greater<double>>
      asks;  // DESC

  market_data()
  {
    reset();
  }
  void reset()
  {
    orders.clear();
    bids.clear();
    asks.clear();
  }
};

class WebSocket : public std::enable_shared_from_this<WebSocket>
{
  binance::http::stream& api_;
  binance::websocket::stream& ws_;
  binance::buffer buffer_;
  binance::json::parser parser_;
  // queued messages
  std::vector<std::string> queued_;
  // market data
  market_data market_;
  std::string topic_;

public:
  WebSocket(binance::http::stream& api, binance::websocket::stream& ws)
      : api_(api)
      , ws_(ws)
      , snapshot_(false)
      , topic_("EOS-ETH")
  {
    using namespace binance::websocket;
    ws_.subscribe(subscribe_to::level3(topic_));
  }

  void start()
  {
    read();
  }

private:
  bool snapshot_;

  void read()
  {
    auto self = shared_from_this();
    buffer_.clear();
    ws_.async_read(buffer_, [this, self](boost::system::error_code ec) {
      if (ec)
        throw ec;

      if (not snapshot_)
      {
        std::cout << "Collecting messages" << std::endl;
        queued_.push_back(boost::beast::buffers_to_string(buffer_.data()));
        if (queued_.size() == 10)
        {
          get_orderbook();
        }
      }
      else
      {
        namespace level3 = binance::websocket::messages::level3;

        const binance::json::object& jb = parser_.parse(buffer_).root();
        binance::websocket::messages::head header(jb);

        if (header.subject == "received")
        {
          level3::received rv(jb["data"]);
          on_received(rv);
        }
        else if (header.subject == "open")
        {
          level3::open rv(jb["data"]);
          on_open(rv);
        }
        else if (header.subject == "done")
        {
          level3::done rv(jb["data"]);
          on_done(rv);
        }
      }

      read();
    });
  }

  void on_received(const binance::websocket::messages::level3::received& rv)
  {
    auto v = std::make_shared<order>(rv);
    market_.orders.insert({v->id, v});
  }

  void on_open(const binance::websocket::messages::level3::open& rv)
  {
    if (rv.price == 0 || rv.size == 0)
      return;

    auto it = market_.orders.find(rv.order_id);
    if (it == market_.orders.end())
    {
      std::cout << "OPEN not found: " << rv.order_id << std::endl;
      return;
    }
    std::cout << "OPEN " << rv.order_id << std::endl;

    auto& v = it->second;
    v->set_side(rv);
    v->price = rv.price;
    v->size  = rv.size;

    if (v->side == order_side::BUY)
      market_.bids.insert({v->price, v});
    else
      market_.asks.insert({v->price, v});
  }

  void on_done(const binance::websocket::messages::level3::done& rv)
  {
    auto it = market_.orders.find(rv.order_id);
    if (it == market_.orders.end())
    {
      std::cout << "DONE order not found: " << rv.order_id << " / " << rv.reason
                << std::endl;
      return;
    }

    auto& v = it->second;
    if (v->side == order_side::BUY)
    {
      auto nit = market_.bids.find(v->price);
      while (nit != market_.bids.end())
      {
        if (*nit->second == *v)
        {
          market_.bids.erase(nit);
          break;
        }
        ++nit;
      }
    }
    else
    {
      auto nit = market_.asks.find(v->price);
      while (nit != market_.asks.end())
      {
        if (*nit->second == *v)
        {
          market_.asks.erase(nit);
          break;
        }
        ++nit;
      }
    }

    std::cout << "DONE: " << it->second->id << " " << rv.reason << std::endl;

    market_.orders.erase(it);
  }

  void get_orderbook()
  {
    api_.async_read<binance::http::messages::orderbook_atomic>(
        [this](const binance::http::messages::orderbook_atomic& ob) {
          std::cout << ob.bids.size() << " " << ob.asks.size() << std::endl;
          for (auto& bid : ob.bids)
          {
            auto v = std::make_shared<order>(bid, true);
            market_.bids.insert({v->price, v});
            market_.orders.insert({v->id, v});
          }

          for (auto& ask : ob.asks)
          {
            auto v = std::make_shared<order>(ask, false);
            market_.asks.insert({v->price, v});
            market_.orders.insert({v->id, v});
          }

          namespace level3 = binance::websocket::messages::level3;

          for (auto& s : queued_)
          {
            const binance::json::object& jb = parser_.parse(s).root();
            binance::websocket::messages::head header(jb);
            auto& subject = header.subject;

            if (subject == "received")
            {
              level3::received rv(jb["data"]);
              if (rv.sequence <= ob.sequence)
                continue;

              auto v = std::make_shared<order>(rv);
              market_.orders.insert({v->id, v});
            }
            else if (subject == "open")
            {
              level3::open rv(jb["data"]);
              if (rv.sequence <= ob.sequence)
                continue;

              auto it = market_.orders.find(rv.order_id);
              if (it == market_.orders.end())
              {
                std::cout << "OPEN ORDER " << rv.order_id
                          << " NOT FOUND BUILDING THE ORDERBOOK" << std::endl;
                continue;
              }
              auto& v = it->second;
              v->set_side(rv);
              v->price = rv.price;
              v->size  = rv.size;

              if (it->second->side == order_side::BUY)
                market_.bids.insert({v->price, v});
              else
                market_.asks.insert({v->price, v});
            }
            else if (subject == "done")
            {
              level3::done rv(jb["data"]);
              if (rv.sequence <= ob.sequence)
                continue;

              auto it = market_.orders.find(rv.order_id);
              if (it == market_.orders.end())
              {
                std::cout << "DONE ORDER " << rv.order_id
                          << " NOT FOUND BUILDING THE ORDERBOOK" << std::endl;
                continue;
              }

              auto& v = it->second;
              if (v->side == order_side::BUY)
              {
                auto nit = market_.bids.find(v->price);
                while (nit != market_.bids.end())
                {
                  if (*nit->second == *v)
                  {
                    market_.bids.erase(nit);
                    break;
                  }
                  ++nit;
                }
              }
              else
              {
                auto nit = market_.asks.find(v->price);
                while (nit != market_.asks.end())
                {
                  if (*nit->second == *v)
                  {
                    market_.asks.erase(nit);
                    break;
                  }
                  ++nit;
                }
              }

              market_.orders.erase(it);
            }
          }
          queued_.clear();
          snapshot_ = true;
        },
        topic_);
  }
};

int main(int argc, char* argv[])
{
  boost::program_options::variables_map args;
  boost::program_options::options_description desc(argv[0]);

  std::cout << "Using Binance version " << KUCOIN_VERSION << std::endl;

  parse_args(argc, argv, desc, args);
  if (args.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }

  binance::io_context ioc;
  binance::http::stream api(
      ioc,
      {args["key"].as<std::string>(), args["secret"].as<std::string>(),
       args["passphrase"].as<std::string>()},
      args["url"].as<std::string>());

  std::cout << "Connecting to " << args["url"].as<std::string>() << std::endl;
  api.async_connect();

  // the ping interval will occur every `ping_interval` - 200.
  // systems with <=200ms latency will work ok.
  //
  // is the same as explicitly doing:
  // opts.ping_interval -= 200;
  binance::websocket::stream ws(ioc);

  api.async_read<binance::http::messages::websocket_token>([&](const auto& tok) {
    binance::websocket::options opts(tok, 200);

    ws.connect(tok);

    std::make_shared<WebSocket>(api, ws)->start();
  });

  // handle signals and stop processing when SIGINT is received
  boost::asio::signal_set signals(ioc, SIGINT);
  signals.async_wait([&](const boost::system::error_code& ec, int n) {
    boost::ignore_unused(ec);
    boost::ignore_unused(n);
    ioc.stop();
  });

  try
  {
    ioc.run();
  }
  catch (const binance::error& ec)
  {
    std::cout << "error: " << ec << std::endl;
    exit(1);
  }

  return 0;
}
