#include <algorithm>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <config.hpp>
#include <iostream>
#include <binance.hpp>

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
  binance::http::stream api(ioc, {cnf.api_key, cnf.api_secret},
                           args["url"].as<std::string>());

  std::cout << "Connecting to " << args["url"].as<std::string>() << std::endl;

  api.async_connect();
  api.async_read<binance::http::messages::get_position_mode>(
      [&api](const auto& k) {
        std::cout << k.dual_position << std::endl;
        api.close();
      });

  try
  {
    ioc.run();
  }
  catch (const binance::error& ec)
  {
    std::cout << ec << std::endl;
  }

  return 0;
}
