#ifndef BINANCE_DEFINITIONS_HPP
#define BINANCE_DEFINITIONS_HPP

#include <boost/preprocessor/stringize.hpp>

constexpr auto BINANCE_DEFAULT_URL = "https://fapi.binance.com";
constexpr auto BINANCE_WS_HOST     = "fstream.binance.com";

#define BINANCE_FUTURES_VERSION "0.1"
#define BINANCE_VERSION_STRING "binance-futures-sdk/0.1"

namespace binance
{
const char* order_side_string[] = {"BUY", "SELL"};
enum order_side
{
  BUY  = 0,
  SELL = 1,
};
const char* order_status_string[] = {"NEW",      "PARTIALLY_FILLED", "FILLED",
                                     "CANCELED", "REJECTED",         "EXPIRED"};
enum order_status
{
  NEW              = 0,
  PARTIALLY_FILLED = 1,
  FILLED           = 2,
  CANCELED         = 3,
  REJECTED         = 4,
  EXPIRED          = 5
};
const char* order_type_string[] = {"LIMIT",
                                   "MARKET",
                                   "STOP",
                                   "STOP_MARKET",
                                   "TAKE_PROFIT",
                                   "TAKE_PROFIT_MARKET",
                                   "TRAILING_STOP_MARKET"};
enum order_type
{
  LIMIT                = 0,
  MARKET               = 1,
  STOP                 = 2,
  STOP_MARKET          = 3,
  TAKE_PROFIT          = 4,
  TAKE_PROFIT_MARKET   = 5,
  TRAILING_STOP_MARKET = 6
};
const char* position_side_string[] = {"BOTH", "LONG", "SHORT"};
enum position_side
{
  BOTH  = 0,
  LONG  = 1,
  SHORT = 2
};
const char* time_in_force_string[] = {"GTC", "IOC", "FOK", "GTX"};
enum time_in_force
{
  // Good Till Cancel
  GTC = 0,
  // Immediate or Cancel
  IOC = 1,
  // Fill or Kill
  FOK = 2,
  // Good Till Crossing (Post only)
  GTX = 3
};
const char* working_type_string[] = {"MARK_PRICE", "CONTRACT_PRICE"};
enum working_type
{
  MARK_PRICE     = 0,
  CONTRACT_PRICE = 1
};
const char* response_type_string[] = {"ACK", "RESULT"};
enum response_type
{
  ACK    = 0,
  RESULT = 1
};
}  // namespace binance

#endif