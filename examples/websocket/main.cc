#include <binance.hpp>
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
      "config,c", opt::value<std::string>()->default_value("binance.conf"),
      "Binance configuration file")(
      "url,U", opt::value<std::string>()->default_value(BINANCE_DEFAULT_URL),
      "Binance API base URL");

  opt::store(opt::parse_command_line(argc, argv, desc), vm);
  opt::notify(vm);
}

class WebSocket : public std::enable_shared_from_this<WebSocket>
{
  binance::websocket::stream& ws_;
  binance::buffer buffer_;
  binance::json::parser parser_;

public:
  WebSocket(binance::websocket::stream& ws)
      : ws_(ws)
  {
    using namespace binance::websocket;
    ws_.subscribe(subscribe_to::kline("btcusdt", "1m"));
  }

  void start()
  {
    read();
  }

private:
  void read()
  {
    auto self = shared_from_this();
    buffer_.clear();
    ws_.async_read(buffer_, [this, self](boost::system::error_code ec) {
      if (ec)
        throw ec;

      binance::websocket::messages::kline kl;
      const binance::json::object& data = parser_.parse(buffer_).root();
      auto k                            = data["k"];
      if (k.error() == simdjson::SUCCESS)
      {
        kl = k;
        std::cout << (kl.closed ? "CLOSED: " : "OPEN: ") << kl.open_price
                  << " | " << kl.trades << std::endl;
      }

      read();
    });
  }
};

int main(int argc, char* argv[])
{
  boost::program_options::variables_map args;
  boost::program_options::options_description desc(argv[0]);

  std::cout << "Using Binance version " << BINANCE_VERSION << std::endl;

  parse_args(argc, argv, desc, args);
  if (args.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }

  config cnf = config_parser::parse_file(args["config"].as<std::string>());

  binance::io_context ioc;
  binance::websocket::stream ws(ioc);

  // handle signals and stop processing when SIGINT is received
  boost::asio::signal_set signals(ioc, SIGINT);
  signals.async_wait([&](const boost::system::error_code& ec, int n) {
    boost::ignore_unused(ec);
    boost::ignore_unused(n);
    ioc.stop();
  });

  try
  {
    ws.connect();

    std::make_shared<WebSocket>(ws)->start();

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
