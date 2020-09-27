#ifndef BINANCE_DEFINITIONS_HPP
#define BINANCE_DEFINITIONS_HPP

#include <boost/preprocessor/stringize.hpp>

constexpr auto BINANCE_DEFAULT_URL = "https://fapi.binance.com";
constexpr auto BINANCE_WS_HOST      = "fstream.binance.com";

#define BINANCE_VERSION_STRING "binance-futures-sdk/" BOOST_STRINGIZE(BINANCE_VERSION)

#endif