#ifndef BINANCE_HTTP_STREAM_HPP
#define BINANCE_HTTP_STREAM_HPP

#include <binance/auth.hpp>
#include <binance/common.hpp>
#include <binance/crypto/signer.hpp>
#include <binance/definitions.hpp>
#include <binance/error.hpp>
#include <binance/http/messages.hpp>
#include <binance/json.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>
#include <boost/url.hpp>
#include <boost/variant2/variant.hpp>
#include <list>
#include <utility>
#ifdef BINANCE_DEBUG
#include <iostream>
#endif

namespace binance
{
namespace http
{
class __request_elem;
// TODO:
class __request_visitor
{
  __request_elem& e_;
  const json::value& jv_;

public:
  __request_visitor(__request_elem& e, const json::value& jv)
      : e_(e)
      , jv_(jv)
  {
  }
};

template<typename Arg>
struct _function : public std::function<void(Arg)>
{
  using arg_type = Arg;
  _function(std::function<void(Arg)>& cb)
      : std::function<void(Arg)>(cb)
  {
  }
  _function(std::function<void(Arg)>&& cb)
      : std::function<void(Arg)>(cb)
  {
  }
};

struct __request_elem
{
  boost::variant2::variant<
      std::shared_ptr<
          boost::beast::http::request<boost::beast::http::empty_body>>,
      std::shared_ptr<
          boost::beast::http::request<boost::beast::http::string_body>>>
      req_;
  void* message_;
  boost::variant2::variant<
      _function<messages::get_position_mode*>, _function<messages::kline_data*>,
      _function<messages::listen_key*>, _function<messages::empty_args*>,
      _function<messages::place_order*>, _function<messages::cancel_order*>,
      _function<messages::orderbook*>, _function<messages::exchange_info*>,
      _function<messages::recent_trades*>, _function<messages::mark_price*>,
      _function<messages::price_ticker*>,
      _function<messages::cancel_order_all*>,
      _function<messages::current_open_order*>,
      _function<messages::current_open_order_all*>>
      cb_;

  template<typename Body, typename T>
  explicit __request_elem(
      std::shared_ptr<boost::beast::http::request<Body>> req, T* tok,
      std::function<void(T*)> cb)
      : req_(req)
      , message_(tok)
      , cb_(cb)
  {
  }
  __request_elem(const __request_elem&) = default;

  void operator()(const json::value& jv)
  {
    boost::variant2::visit(
        [this, &jv](auto cb) {
          auto v = static_cast<typename decltype(cb)::arg_type>(message_);
          *v     = jv;
          cb(v);
        },
        cb_);
  }
};

template<typename T>
using DefaultHandler = std::function<void(T*)>;

// TODO: Maybe add compatibility with Boost 1.67 ????
using http_stream_t =
    std::optional<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;

enum __SECURITY_CODES
{
  NONE,
  TRADE,
  USER_DATA,
  USER_STREAM,
  MARKET_DATA
};

class stream
{
  binance::io_context& ioc_;
  boost::asio::ssl::context ctx_;
  http_stream_t stream_;
  boost::urls::url base_url_;
  boost::asio::deadline_timer timeout_;
  boost::asio::ip::tcp::resolver::results_type resolve_results_;
  auth_opts auth_;
  // TODO: One buffer and one parser per stream?
  boost::beast::flat_buffer buffer_;
  json::parser parser_;

  std::vector<boost::asio::deadline_timer> timers_;

  bool is_open_;
  std::list<__request_elem> queue_;
  boost::beast::http::response<boost::beast::http::string_body> response_;
  bool is_writing_;

  size_t req_count_;
  size_t rate_limit_;
  boost::posix_time::seconds limit_window_;

public:
  stream()               = delete;
  stream(const stream&)  = delete;
  stream(const stream&&) = delete;  // TODO: Enable?
  stream(binance::io_context& ioc, auth_opts opts = {},
         const std::string& base_url = BINANCE_DEFAULT_URL,
         boost::asio::ssl::context::method method =
             boost::asio::ssl::context::tlsv13_client);
  // Connection will automatically close on destruction.
  ~stream();
  // reset ...
  void reset();
  // close ...
  void close();
  bool is_open() const;
  bool is_busy() const;
  void discard_next();
  void set_rate_limit(size_t limit, boost::posix_time::seconds window);
  void async_connect();
  template<typename T, class... Args>
  void async_read(DefaultHandler<T>, Args... args);
  template<typename T, class... Args>
  void async_write(DefaultHandler<T>, Args... args);

  void async_read(messages::get_position_mode*,
                  DefaultHandler<messages::get_position_mode>);
  void async_read(messages::listen_key*, DefaultHandler<messages::listen_key>);
  void async_read(messages::exchange_info*,
                  DefaultHandler<messages::exchange_info>);
  void async_read(messages::orderbook*, DefaultHandler<messages::orderbook>);
  void async_read(messages::recent_trades*,
                  DefaultHandler<messages::recent_trades>);
  // https://binance-docs.github.io/apidocs/futures/en/#old-trades-lookup-market_data
  // https://binance-docs.github.io/apidocs/futures/en/#compressed-aggregate-trades-list
  void async_read(messages::kline_data*, DefaultHandler<messages::kline_data>);
  void async_read(messages::mark_price*, DefaultHandler<messages::mark_price>);
  // https://binance-docs.github.io/apidocs/futures/en/#get-funding-rate-history
  // https://binance-docs.github.io/apidocs/futures/en/#24hr-ticker-price-change-statistics
  void async_read(messages::price_ticker*,
                  DefaultHandler<messages::price_ticker>);
  // https://binance-docs.github.io/apidocs/futures/en/#symbol-order-book-ticker
  // https://binance-docs.github.io/apidocs/futures/en/#get-all-liquidation-orders
  // https://binance-docs.github.io/apidocs/futures/en/#open-interest
  // https://binance-docs.github.io/apidocs/futures/en/#open-interest-statistics
  // https://binance-docs.github.io/apidocs/futures/en/#top-trader-long-short-ratio-accounts-market_data
  // https://binance-docs.github.io/apidocs/futures/en/#top-trader-long-short-ratio-positions
  // https://binance-docs.github.io/apidocs/futures/en/#long-short-ratio
  // https://binance-docs.github.io/apidocs/futures/en/#taker-buy-sell-volume
  // https://binance-docs.github.io/apidocs/futures/en/#historical-blvt-nav-kline-candlestick

  // https://binance-docs.github.io/apidocs/futures/en/#new-future-account-transfer
  // https://binance-docs.github.io/apidocs/futures/en/#get-future-account-transaction-history-list-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#change-position-mode-trade
  // https://binance-docs.github.io/apidocs/futures/en/#get-current-position-mode-user_data
  void async_write(messages::place_order*,
                   DefaultHandler<messages::place_order>);
  // https://binance-docs.github.io/apidocs/futures/en/#place-multiple-orders-trade
  // https://binance-docs.github.io/apidocs/futures/en/#query-order-user_data
  void async_write(messages::cancel_order*,
                   DefaultHandler<messages::cancel_order>);
  void async_write(messages::cancel_order_all*,
                   DefaultHandler<messages::cancel_order_all>);
  // https://binance-docs.github.io/apidocs/futures/en/#cancel-multiple-orders-trade
  // https://binance-docs.github.io/apidocs/futures/en/#auto-cancel-all-open-orders-trade
  void async_read(messages::current_open_order*,
                  DefaultHandler<messages::current_open_order>);
  void async_read(messages::current_open_order_all*,
                  DefaultHandler<messages::current_open_order_all>);
  // https://binance-docs.github.io/apidocs/futures/en/#all-orders-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#futures-account-balance-v2-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#account-information-v2-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#change-initial-leverage-trade
  // https://binance-docs.github.io/apidocs/futures/en/#change-margin-type-trade
  // https://binance-docs.github.io/apidocs/futures/en/#modify-isolated-position-margin-trade
  // https://binance-docs.github.io/apidocs/futures/en/#get-position-margin-change-history-trade
  // https://binance-docs.github.io/apidocs/futures/en/#position-information-v2-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#account-trade-list-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#get-income-history-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#notional-and-leverage-brackets-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#position-adl-quantile-estimation-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#user-39-s-force-orders-user_data
  // https://binance-docs.github.io/apidocs/futures/en/#user-api-trading-quantitative-rules-indicators-user_data

  // renew_listen_key sets up a timer to renew the listen_key automatically
  // for the WebSocket User Data Streams.
  void renew_listen_key();

private:
  // ping every 15 seconds.
  void ping_timer();
  void rate_timer();
  // clear all the timers that expired
  void clear_timers();
  void on_connect(boost::system::error_code const&,
                  const boost::asio::ip::tcp::endpoint&);
  void on_write(boost::system::error_code const&, size_t);
  void on_read(boost::system::error_code const&, size_t);
  really_inline void next_async_request();
  template<class T>
  really_inline void do_write(boost::beast::http::request<T>&);
  template<class ReqBody, class Msg, __SECURITY_CODES C>
  void async_call(std::shared_ptr<boost::beast::http::request<ReqBody>> req,
                  Msg* msg, DefaultHandler<Msg> cb);
  template<class ReqBody, __SECURITY_CODES C, class Msg>
  void async_get(const std::string& endpoint, Msg* msg, DefaultHandler<Msg> cb);
  template<class ReqBody, __SECURITY_CODES C, class Msg>
  void async_post(const std::string& endpoint, Msg* msg,
                  DefaultHandler<Msg> cb);
  template<class ReqBody, __SECURITY_CODES C, class Msg>
  void async_del(const std::string& endpoint, Msg* msg, DefaultHandler<Msg> cb);
  template<class ReqBody, __SECURITY_CODES C, class Msg>
  void async_put(const std::string& endpoint, Msg* msg, DefaultHandler<Msg> cb);
  template<class BodyType, class Msg, __SECURITY_CODES C>
  void prepare_request(boost::beast::http::request<BodyType>&, Msg* msg);
  really_inline void get_error_codes(binance::error&, const json::value&);
  template<class JSONValue>
  really_inline void parse_response(binance::error& ec, JSONValue& v,
                                    const std::string& body);
  really_inline void async_ping();
  really_inline void enable_writing();
  really_inline void disable_writing();
};

stream::stream(binance::io_context& ioc, auth_opts opts,
               const std::string& base_url,
               boost::asio::ssl::context::method method)
    : ioc_(ioc)
    , timeout_(ioc)
    , base_url_(base_url)
    , ctx_(method)
    , auth_(opts)
    , is_open_(false)
    , is_writing_(false)
    , rate_limit_(0)
    , req_count_(0)
    , limit_window_(0)
{
}

stream::~stream()
{
  // avoid exceptions using error_code
  if (stream_)
    close();
}

void stream::set_rate_limit(size_t limit, boost::posix_time::seconds window)
{
  rate_limit_   = limit;
  limit_window_ = window;
  rate_timer();
}

void stream::close()
{
  binance::boost_error ec;

  stream_->shutdown(ec);
  stream_->next_layer().close();
  timers_.clear();

  is_open_ = false;
}

void stream::reset()
{
  if (stream_)
    close();
  stream_.emplace(ioc_, ctx_);
}

bool stream::is_busy() const
{
  return is_writing_;
}

void stream::discard_next()
{
  if (!queue_.empty())
    queue_.pop_front();
}

bool stream::is_open() const
{
  return stream_ && stream_->next_layer().is_open() && is_open_;
}

void stream::clear_timers()
{
  for (auto it = timers_.begin(); it != timers_.end();)
  {
    if (it->expires_at() < boost::posix_time::second_clock::local_time())
    {
      timers_.erase(it);
      it = timers_.begin();
    }
    else
      ++it;
  }
}

void stream::disable_writing()
{
  is_writing_ = false;
  timeout_.cancel();
}

void stream::enable_writing()
{
  is_writing_ = true;
  req_count_++;

  timeout_.cancel();
  timeout_.expires_from_now(boost::posix_time::seconds(15));
  timeout_.async_wait([&](boost::system::error_code ec) {
    if (!ec && is_writing_)
      stream_->next_layer().cancel();
  });
}

void stream::async_connect()
{
  reset();

  std::string host = base_url_.host();
  std::string port = base_url_.port().to_string();
  if (port.empty())
    port = "443";

  ctx_.set_verify_mode(boost::asio::ssl::verify_none);

  if (!::SSL_set_tlsext_host_name(stream_->native_handle(), host.c_str()))
  {
    boost::beast::error_code ec{static_cast<int>(::ERR_get_error()),
                                boost::asio::error::get_ssl_category()};
    throw binance::error{ec};
  }

  if (resolve_results_.empty())
  {
    boost::asio::ip::tcp::resolver resolver(ioc_);
    resolve_results_ = resolver.resolve(host, port);
  }

  using std::placeholders::_1;
  using std::placeholders::_2;

  boost::asio::async_connect(boost::beast::get_lowest_layer(*stream_),
                             resolve_results_,
                             std::bind(&stream::on_connect, this, _1, _2));
}

void stream::renew_listen_key()
{
  timers_.emplace_back(ioc_);

  auto& timer = timers_.back();
  timer.expires_from_now(boost::posix_time::minutes(59));
  timer.async_wait([this](boost::system::error_code ec) {
    if (ec)
      return;

    namespace http = boost::beast::http;

    messages::empty_args* e                = new messages::empty_args();
    DefaultHandler<messages::empty_args> f = [this](messages::empty_args* e) {
      delete e;
      renew_listen_key();
    };

    async_put<http::empty_body, __SECURITY_CODES::USER_DATA>(
        "/fapi/v1/listenKey", e, std::move(f));
    clear_timers();
  });
}

void stream::ping_timer()
{
  timers_.emplace_back(ioc_);

  auto& timer = timers_.back();
  timer.expires_from_now(boost::posix_time::seconds(15));
  timer.async_wait([this](boost::system::error_code ec) {
    if (ec)
      return;

    clear_timers();

    if (is_writing_)
      ping_timer();
    else
      async_ping();
  });
}

void stream::rate_timer()
{
  timers_.emplace_back(ioc_);

  auto& timer = timers_.back();
  timer.expires_from_now(limit_window_);
  timer.async_wait([this](boost::system::error_code ec) {
    if (ec)
      return;

    clear_timers();

    if (rate_limit_ > 0)
      rate_timer();
    req_count_ = 0;
    next_async_request();
  });
}

void stream::on_connect(boost::system::error_code const& ec,
                        const boost::asio::ip::tcp::endpoint& endpoint)
{
  if (ec)
    throw ec;
  is_writing_ = false;

  stream_->handshake(boost::asio::ssl::stream_base::client);

  is_open_ = true;
  stream_->next_layer().non_blocking(true);

  next_async_request();
  ping_timer();
}

void stream::on_write(boost::system::error_code const& ec, size_t size)
{
  namespace http = boost::beast::http;
  boost::ignore_unused(size);
  if (ec)
  {
    disable_writing();
    throw ec;
  }

  response_.clear();
  response_.body().clear();
  http::async_read(*stream_, buffer_, response_,
                   boost::beast::bind_front_handler(&stream::on_read, this));
}

// if an exception happens here the programmer should call discard_next
// in case the request can be skipped and not re-tried.
void stream::on_read(boost::system::error_code const& ec, size_t size)
{
  boost::ignore_unused(size);
  namespace http = boost::beast::http;

  if (ec)
  {
    disable_writing();
    throw ec;
  }

  auto& res = response_;
  if (res.result_int() != 200)
  {
    disable_writing();
    throw binance::error{res.result_int(), res.body()};
  }

  auto& e = queue_.front();

  binance::error err;
  const json::value& v = parser_.parse(res.body()).root();

  if (v.is_object())
  {
    get_error_codes(err, v);
    if (err)
    {
      disable_writing();
      throw err;
    }
  }

  e(v);

  queue_.pop_front();
  disable_writing();

  auto it = res.base().find("Connection");
  if (it != res.base().end() && it->value() == "close")
  {
    async_connect();  // reconnect
  }
  else
  {
    next_async_request();
  }
}

really_inline void stream::next_async_request()
{
  if (not is_open() || queue_.empty() || is_writing_
      || (rate_limit_ > 0 && req_count_ >= rate_limit_))
    return;

  auto& e = queue_.front();
  boost::variant2::visit([this](auto req) { do_write(*req); }, e.req_);
}

template<class T>
really_inline void stream::do_write(boost::beast::http::request<T>& req)
{
  enable_writing();
  boost::beast::http::async_write(
      *stream_, req, boost::beast::bind_front_handler(&stream::on_write, this));
}

template<class ReqBody, class Msg, __SECURITY_CODES C>
void stream::async_call(
    std::shared_ptr<boost::beast::http::request<ReqBody>> req, Msg* msg,
    DefaultHandler<Msg> cb)
{
  namespace http = boost::beast::http;
#ifdef BINANCE_DEBUG
  std::cout << "REQ: " << req->base().target();
  if constexpr (!std::is_same_v<ReqBody, http::empty_body>)
    std::cout << "\n" << req->body() << std::endl;
  std::cout << std::endl;
#endif
  prepare_request<ReqBody, Msg, C>(*req, msg);
  if constexpr (!std::is_same_v<ReqBody, http::empty_body>)
    req->set(http::field::content_length, req->body().size());

  queue_.emplace_back(req, msg, std::move(cb));

  next_async_request();
}

template<class ReqBody, __SECURITY_CODES C, class Msg>
void stream::async_get(const std::string& endpoint, Msg* msg,
                       DefaultHandler<Msg> cb)
{
  namespace http = boost::beast::http;
  // TODO: Remove the pointer from here
  auto req =
      std::make_shared<http::request<ReqBody>>(http::verb::get, endpoint, 11);

  async_call<ReqBody, Msg, C>(req, msg, cb);
}

template<class ReqBody, __SECURITY_CODES C, class Msg>
void stream::async_put(const std::string& endpoint, Msg* msg,
                       DefaultHandler<Msg> cb)
{
  namespace http = boost::beast::http;
  auto req =
      std::make_shared<http::request<ReqBody>>(http::verb::put, endpoint, 11);

  async_call<ReqBody, Msg, C>(req, msg, cb);
}

template<class ReqBody, __SECURITY_CODES C, class Msg>
void stream::async_del(const std::string& endpoint, Msg* msg,
                       DefaultHandler<Msg> cb)
{
  namespace http = boost::beast::http;
  auto req       = std::make_shared<http::request<ReqBody>>(http::verb::delete_,
                                                      endpoint, 11);

  async_call<ReqBody, Msg, C>(req, msg, cb);
}

template<class ReqBody, __SECURITY_CODES C, class Msg>
void stream::async_post(const std::string& endpoint, Msg* msg,
                        DefaultHandler<Msg> cb)
{
  namespace http = boost::beast::http;
  auto req =
      std::make_shared<http::request<ReqBody>>(http::verb::post, endpoint, 11);

  async_call<ReqBody, Msg, C>(req, msg, cb);
}

template<typename T, class... Args>
void stream::async_read(DefaultHandler<T> cb, Args... args)
{
  auto v = std::make_shared<T>(std::forward<Args>(args)...);
  async_read(v.get(), [v, cb](T* p) { cb(p); });
}

template<typename T, class... Args>
void stream::async_write(DefaultHandler<T> cb, Args... args)
{
  auto v = std::make_shared<T>(std::forward<Args>(args)...);
  async_write(v.get(), [v, cb](T* p) { cb(p); });
}

void stream::async_read(messages::get_position_mode* msg,
                        DefaultHandler<messages::get_position_mode> cb)
{
  namespace http = boost::beast::http;
  msg->insert_kv({"timestamp", string_milli_epoch()});
  async_get<http::empty_body, __SECURITY_CODES::USER_DATA>(
      "/fapi/v1/positionSide/dual", msg, std::move(cb));
}

void stream::async_read(messages::kline_data* msg,
                        DefaultHandler<messages::kline_data> cb)
{
  namespace http = boost::beast::http;
  async_get<http::empty_body, __SECURITY_CODES::NONE>("/fapi/v1/klines", msg,
                                                      std::move(cb));
}

void stream::async_read(messages::listen_key* msg,
                        DefaultHandler<messages::listen_key> cb)
{
  namespace http = boost::beast::http;
  async_post<http::empty_body, __SECURITY_CODES::USER_STREAM>(
      "/fapi/v1/listenKey", msg, std::move(cb));
}

void stream::async_write(messages::place_order* msg,
                         DefaultHandler<messages::place_order> cb)
{
  namespace http = boost::beast::http;
  msg->insert_kv({"timestamp", string_milli_epoch()});
  async_post<http::string_body, __SECURITY_CODES::TRADE>("/fapi/v1/order", msg,
                                                         std::move(cb));
}

void stream::async_write(messages::cancel_order* msg,
                         DefaultHandler<messages::cancel_order> cb)
{
  namespace http = boost::beast::http;
  msg->insert_kv({"timestamp", string_milli_epoch()});
  async_del<http::string_body, __SECURITY_CODES::TRADE>("/fapi/v1/order", msg,
                                                        std::move(cb));
}

void stream::async_write(messages::cancel_order_all* msg,
                         DefaultHandler<messages::cancel_order_all> cb)
{
  namespace http = boost::beast::http;
  msg->insert_kv({"timestamp", string_milli_epoch()});
  async_del<http::string_body, __SECURITY_CODES::TRADE>(
      "/fapi/v1/allOpenOrders", msg, std::move(cb));
}

void stream::async_read(messages::current_open_order* msg,
                        DefaultHandler<messages::current_open_order> cb)
{
  namespace http = boost::beast::http;
  msg->insert_kv({"timestamp", string_milli_epoch()});
  async_get<http::empty_body, __SECURITY_CODES::USER_DATA>("/fapi/v1/openOrder",
                                                           msg, std::move(cb));
}

void stream::async_read(messages::current_open_order_all* msg,
                        DefaultHandler<messages::current_open_order_all> cb)
{
  namespace http = boost::beast::http;
  msg->insert_kv({"timestamp", string_milli_epoch()});
  async_get<http::empty_body, __SECURITY_CODES::USER_DATA>("/fapi/v1/allOrders",
                                                           msg, std::move(cb));
}

void stream::async_read(messages::exchange_info* msg,
                        DefaultHandler<messages::exchange_info> cb)
{
  namespace http = boost::beast::http;
  async_get<http::empty_body, __SECURITY_CODES::NONE>("/fapi/v1/exchangeInfo",
                                                      msg, std::move(cb));
}

void stream::async_read(messages::orderbook* msg,
                        DefaultHandler<messages::orderbook> cb)
{
  namespace http = boost::beast::http;
  async_get<http::empty_body, __SECURITY_CODES::NONE>("/fapi/v1/depth", msg,
                                                      std::move(cb));
}

void stream::async_read(messages::recent_trades* msg,
                        DefaultHandler<messages::recent_trades> cb)
{
  namespace http = boost::beast::http;
  async_get<http::empty_body, __SECURITY_CODES::NONE>("/fapi/v1/klines", msg,
                                                      std::move(cb));
}

void stream::async_read(messages::mark_price* msg,
                        DefaultHandler<messages::mark_price> cb)
{
  namespace http = boost::beast::http;
  async_get<http::empty_body, __SECURITY_CODES::NONE>("/fapi/v1/premiumIndex",
                                                      msg, std::move(cb));
}

void stream::async_read(messages::price_ticker* msg,
                        DefaultHandler<messages::price_ticker> cb)
{
  namespace http = boost::beast::http;
  async_get<http::empty_body, __SECURITY_CODES::NONE>("/fapi/v1/ticker/price",
                                                      msg, std::move(cb));
}

void stream::async_ping()
{
  namespace http = boost::beast::http;
  auto v         = std::make_shared<messages::empty_args>();
  async_get<http::empty_body, __SECURITY_CODES::NONE, messages::empty_args>(
      "/fapi/v1/ping", v.get(), [this, v](auto* e) {
        boost::ignore_unused(e);
        ping_timer();
      });
}

really_inline void stream::get_error_codes(binance::error& ec,
                                           const json::value& jv)
{
  int code = 0;
  json::value_to(jv, "code", code);

  ec = code;
  if (ec)
  {
    json::value_to(jv, "msg", ec);
    return;
  }
}

template<class JSONValue>
really_inline void stream::parse_response(binance::error& ec, JSONValue& v,
                                          const std::string& body)
{
  const json::object& jb = parser_.parse(body).root();

  int64_t code = 0;
  json::value_to(jb, "code", code);

  ec = int(code);
  if (ec)
  {
    json::value_to(jb, "msg", ec);
    return;
  }

  json::value_to(jb, "data", v);
}

template<class BodyType, class Msg, __SECURITY_CODES C>
void stream::prepare_request(boost::beast::http::request<BodyType>& req,
                             Msg* msg)
{
  namespace http = boost::beast::http;

  req.set(http::field::host, base_url_.host());
  req.set(http::field::user_agent, BINANCE_VERSION_STRING);
  req.set(http::field::connection, "keep-alive");
  if constexpr (std::is_same<BodyType, http::string_body>::value)
    req.set(http::field::content_type, "application/x-www-form-urlencoded");

  std::string target = req.target().to_string();
  std::string query;

  // if constexpr (std::is_base_of_v<query_args, Msg>)
  if (!msg->args().empty())
  {
    // TODO: Try to parse a copy of the base_url_
    binance::http::parse_args(target, query, msg->args());
  }

  if constexpr (C == __SECURITY_CODES::USER_DATA
                || C == __SECURITY_CODES::TRADE)
  {
    crypto::signer ss(auth_.secret);
    ss.update(query);

    query += "&signature=" + ss.final();
  }

  if constexpr (C == __SECURITY_CODES::USER_DATA || C == __SECURITY_CODES::TRADE
                || C == __SECURITY_CODES::USER_STREAM
                || C == __SECURITY_CODES::MARKET_DATA)
    req.set("X-MBX-APIKEY", auth_.key);

  if constexpr (std::is_same<BodyType, http::empty_body>::value)
    req.target(target + "?" + query);
  else
  {
    req.target(target);
    req.body() = query;
  }
}
}  // namespace http
}  // namespace binance

#endif
