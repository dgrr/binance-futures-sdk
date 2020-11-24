#define BINANCE_WEBSOCKET_ASYNC_CLOSE
#define BINANCE_WEBSOCKET_SHARED_PTR
#include <binance.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <config.hpp>
#include <iostream>
#include <stack>

void parse_args(int argc, char* argv[],
                boost::program_options::options_description& desc,
                boost::program_options::variables_map& vm)
{
  namespace opt = boost::program_options;

  desc.add_options()("help,h", "Help message")(
      "symbol,s", opt::value<std::string>()->default_value("btcusdt"),
      "Symbol to be subscribed to")(
      "url,U", opt::value<std::string>()->default_value(BINANCE_DEFAULT_URL),
      "Binance API base URL");

  opt::store(opt::parse_command_line(argc, argv, desc), vm);
  opt::notify(vm);
}

class WebSocket : public std::enable_shared_from_this<WebSocket>
{
  bool& shutdown_;
  std::string symbol_;
  boost::asio::io_context& ioc_;
  int64_t ts_;

  binance::json::parser parser_;
  std::vector<std::shared_ptr<binance::websocket::stream>> wss_;
  std::stack<binance::buffer*> buffers_;

  bool shutting_down_;

public:
  WebSocket(boost::asio::io_context& ioc, std::string symbol, bool& s) noexcept
      : ioc_(ioc)
      , symbol_(symbol)
      , shutdown_(s)
      , ts_(0)
      , shutting_down_(false)
  {
    wss_.resize(8);
  }

  void start()
  {
    for (auto& ws : wss_)
    {
      ws = std::make_shared<binance::websocket::stream>(ioc_);
      connect(ws);
    }
  }

private:
  void shutdown()
  {
    if (shutting_down_)
      return;
    shutting_down_ = true;

    auto self = shared_from_this();
    for (auto& ws : wss_)
      ws->async_close([self](auto ec) { boost::ignore_unused(ec); });
  }

  void connect(std::shared_ptr<binance::websocket::stream> ws)
  {
    using namespace std::placeholders;
    ws->async_connect(
        std::bind(&WebSocket::on_connect, shared_from_this(), _1, _2));
  }

  void on_connect(std::shared_ptr<binance::websocket::stream> ws,
                  binance::error ec)
  {
    if (ec)
      throw ec;

    ws->subscribe(binance::websocket::subscribe_to::book_ticker(symbol_));

    read(ws);
  }

  void read(std::shared_ptr<binance::websocket::stream> ws)
  {
    if (shutdown_)
      return shutdown();

    // reconnect after 5 minutes
    if (std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - ws->connected_at())
        > std::chrono::minutes(5))
    {
      connect(ws);
      return;
    }

    decltype(buffers_)::value_type buffer = nullptr;
    if (buffers_.empty())
      buffer = new binance::buffer();
    else
    {
      buffer = buffers_.top();
      buffers_.pop();
    }

    buffer->clear();

    using namespace std::placeholders;
    ws->async_read(*buffer, std::bind(&WebSocket::on_read, shared_from_this(),
                                      ws, buffer, _1));
  }

  void on_read(std::shared_ptr<binance::websocket::stream> ws,
               binance::buffer* buffer, boost::system::error_code ec)
  {
    if (ec)
    {
      if (ec == boost::system::errc::operation_canceled)
        return;
      throw ec;
    }

    int64_t ts;
    const binance::json::object& jb = parser_.parse(*buffer).root();

    if (jb["E"].get(ts) == simdjson::SUCCESS && ts_ < ts)
    {
      ts_ = ts;

      binance::websocket::messages::book_ticker bt;
      bt = jb;

      std::cout << bt.best_ask_price << " | " << bt.best_bid_price << std::endl;
    }

    buffers_.push(buffer);

    read(ws);
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

  binance::io_context ioc;
  bool shutdown = false;

  // handle signals and stop processing when SIGINT is received
  boost::asio::signal_set signals(ioc, SIGINT);
  signals.async_wait([&](const boost::system::error_code& ec, int n) {
    boost::ignore_unused(ec);
    boost::ignore_unused(n);
    shutdown = true;
    signals.clear();
  });

  std::string symbol = args["symbol"].as<std::string>();
  try
  {
    std::make_shared<WebSocket>(ioc, symbol, shutdown)->start();

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
