#include <algorithm>
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

class downloader : public std::enable_shared_from_this<downloader>
{
  binance::http::stream& api_;
  binance::http::messages::kline_data kd_;

public:
  downloader(binance::http::stream& api)
      : api_(api)
      , kd_("btcusdt", "1h")
  {
    api_.set_rate_limit(1000, 60);
    kd_.set_limit(1500);
  }
  ~downloader()
  {
    api_.close();
  }
  void run()
  {
    read();
  }

private:
  void read()
  {
    int64_t end_time = 0;
    if (kd_.get("endTime", end_time))
      std::cout << "Getting klines with end_time = " << end_time << std::endl;

    api_.async_read(&kd_, std::bind(&downloader::on_read, shared_from_this(),
                                    std::placeholders::_1));
  }

  void on_read(binance::http::messages::kline_data* kd)
  {
    int64_t diff     = 0;
    int64_t min_time = std::numeric_limits<int64_t>::max();
    for (auto& kl : kd->klines)
      min_time = std::min(min_time, kl.open_time.time_since_epoch().count());

    if (kd->klines.size() > 1)
      diff = kd->klines[1].open_time.time_since_epoch().count()
             - kd->klines[0].open_time.time_since_epoch().count();

    std::cout << "Got " << kd->klines.size() << " klines" << std::endl;
    if (kd->klines.size() == 1500)
    {
      kd_.set_start_time((min_time - diff * 1500) / 1000000);
      kd_.set_end_time(min_time / 1000000);
      read();
    }
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

  std::cout << "Connecting to " << args["url"].as<std::string>() << std::endl;

  api.async_connect();
  std::make_shared<downloader>(api)->run();

  try
  {
    ioc.run();
  }
  catch (const binance::error& ec)
  {
    std::cout << "binance error: " << ec << std::endl;
  }

  return 0;
}
