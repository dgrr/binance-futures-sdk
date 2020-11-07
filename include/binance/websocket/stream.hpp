#ifndef BINANCE_WEBSOCKET_STREAM_HPP
#define BINANCE_WEBSOCKET_STREAM_HPP

#include <binance/common.hpp>
#include <binance/definitions.hpp>
#include <binance/http/messages.hpp>
#include <binance/json.hpp>
#include <binance/websocket/subscribe_to.hpp>
#include <binance/websocket/unsubscribe_from.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/json.hpp>
#include <boost/json/kind.hpp>
#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>
#include <boost/url.hpp>
#include <queue>
#include <type_traits>

namespace binance
{
using buffer = boost::beast::flat_buffer;

namespace websocket
{
// optional for reusability
using websocket_stream_t = std::optional<boost::beast::websocket::stream<
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>>;
class stream
{
  binance::io_context& ioc_;
  boost::asio::ssl::context ctx_;
  websocket_stream_t stream_;
#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
  std::queue<std::string> messages_;
  bool connected_;
#endif
  uint64_t id_;

public:
  // connect_handler will be called when the connection is successfully
  // established.
  using connect_handler = std::function<void(stream*, binance::error)>;

  stream()               = delete;
  stream(const stream&)  = delete;
  stream(const stream&&) = delete;
  stream(binance::io_context& ioc);
  ~stream();

  uint64_t id() const;
  // closes the connection gracefully.
  really_inline void close();
  // async_connect creates a new connection
  //
  // if you call connect with the connection open,
  // the current connection will close.
  void async_connect(connect_handler);
  // async_connect creates a new connection and subscribes
  // the websocket connection to the user data streams
  // using the listen_key.
  void async_connect(const std::string& listen_key, connect_handler);
  // returns true if the stream is open, false otherwise
  really_inline operator bool() const;
  void subscribe(const std::vector<std::string>&);
  // subscribe to a topic
  template<typename... Topic>
  void subscribe(Topic... topics);
  // unsubscribe from a topic
  void unsubscribe(const std::vector<std::string>&);
  template<typename... Topic>
  void unsubscribe(Topic... topics);

  template<class Cb>
  really_inline void async_read(binance::buffer& buffer, Cb&& cb)
  {
    // TODO: Parse JSON here? We can pass the json::object& as argument on the
    // callback.
    stream_->async_read(buffer,
                        [cb](boost::system::error_code ec, std::size_t n) {
                          boost::ignore_unused(n);
                          cb(ec);
                        });
  }

private:
  void async_connect(connect_handler cb, const std::string& endpoint,
                     boost::asio::ssl::verify_mode v_mode);
  void on_connect(connect_handler cb, std::string host, std::string endpoint,
                  boost::asio::ssl::verify_mode v_mode,
                  const boost::system::error_code& ec,
                  const boost::asio::ip::tcp::endpoint& ep);
  void on_ssl_handshake(connect_handler cb, std::string host,
                        std::string endpoint,
                        const boost::system::error_code& ec);
  void on_ws_handshake(connect_handler cb, std::string host,
                       std::string endpoint,
                       const boost::system::error_code& ec);
  void on_control_frame(boost::beast::websocket::frame_type,
                        boost::string_view);
  void on_message_sent(const boost::system::error_code& ec, size_t n);
  void next_async_message();
  really_inline void reset();
};

stream::stream(binance::io_context& ioc)
    : ioc_(ioc)
    , ctx_(boost::asio::ssl::context::tlsv12_client)
    , id_(1)
#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
    , connected_(false)
#endif
{
}

stream::~stream()
{
  if (stream_)
    close();
}

uint64_t stream::id() const
{
  return id_;
}

template<class Topic>
inline constexpr bool topic_constraint =
    std::is_base_of_v<binance::websocket::subscribe_to::topic_path, Topic>;

template<typename... Topic>
void stream::subscribe(Topic... topics)
{
  static_assert((topic_constraint<decltype(topics)> && ...),
                "stream::subscribe only accepts method inheritating from "
                "subscribe_to::topic_path");

  std::vector<std::string> vs;
  vs.resize(sizeof...(topics));

  size_t i = 0;
  ((vs[i++] = topics.topic()), ...);

  subscribe(vs);
}

template<typename... Topic>
void stream::unsubscribe(Topic... topics)
{
  std::vector<std::string> vs;
  vs.resize(sizeof...(topics));

  size_t i = 0;
  ((vs[i++] = topics.topic()), ...);

  unsubscribe(vs);
}

void stream::close()
{
  boost::system::error_code ec;
#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
  connected_ = false;
#endif

  stream_->close(boost::beast::websocket::normal, ec);
  stream_->next_layer().shutdown(ec);
  stream_->next_layer().next_layer().close();
}

void stream::reset()
{
  if (stream_)
    // gracefully close the connection
    close();
  stream_.emplace(ioc_, ctx_);
}

void stream::async_connect(stream::connect_handler cb)
{
  async_connect(cb, "/ws/", boost::asio::ssl::verify_none);
}

void stream::async_connect(const std::string& listen_key,
                           stream::connect_handler cb)
{
  async_connect(cb, "/ws/" + listen_key, boost::asio::ssl::verify_none);
}

void stream::async_connect(stream::connect_handler cb,
                           const std::string& endpoint,
                           boost::asio::ssl::verify_mode v_mode)
{
  reset();

  boost::asio::ip::tcp::resolver resolver{ioc_};
  std::string host = BINANCE_WS_HOST;
  std::string port = "443";

  boost::system::error_code ec;
  auto const results = resolver.resolve(host, port, ec);
  if (ec)
  {
#ifdef BINANCE_DEBUG
    std::cout << "resolve error: " << ec << std::endl;
#endif
    cb(this, ec);
    return;
  }

  using namespace std::placeholders;
  boost::asio::async_connect(
      boost::beast::get_lowest_layer(*stream_), results,
      std::bind(&stream::on_connect, this, cb, host, endpoint, v_mode, _1, _2));
}

void stream::on_connect(stream::connect_handler cb, std::string host,
                        std::string endpoint,
                        boost::asio::ssl::verify_mode v_mode,
                        const boost::system::error_code& ec,
                        const boost::asio::ip::tcp::endpoint& ep)
{
  boost::ignore_unused(ep);
  if (ec)
  {
#ifdef BINANCE_DEBUG
    std::cout << "error connecting to " << host << ":" << port << ": " << ec
              << std::endl;
#endif
    throw binance::error{ec};
  }

  stream_->next_layer().set_verify_mode(v_mode);
  if (!::SSL_set_tlsext_host_name(stream_->next_layer().native_handle(),
                                  host.c_str()))
  {
    cb(this, boost::system::error_code{static_cast<int>(::ERR_get_error()),
                                       boost::asio::error::get_ssl_category()});
    return;
  }

  using namespace std::placeholders;
  stream_->next_layer().async_handshake(
      boost::asio::ssl::stream_base::client,
      std::bind(&stream::on_ssl_handshake, this, cb, host, endpoint, _1));
}

void stream::on_ssl_handshake(stream::connect_handler cb, std::string host,
                              std::string endpoint,
                              const boost::system::error_code& ec)
{
  if (ec)
  {
#ifdef BINANCE_DEBUG
    std::cout << "SSL handshake error: " << ec << std::endl;
#endif
    cb(this, ec);
    return;
  }

  // if (!opts.disable_compression)
  // {
  //   websocket::permessage_deflate msg_def;
  //   msg_def.client_enable = true;
  //   stream_->set_option(msg_def);
  // }
  stream_->set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::request_type& req) {
        req.set(boost::beast::http::field::user_agent, BINANCE_VERSION_STRING);
      }));

  stream_->async_handshake(host, endpoint,
                           std::bind(&stream::on_ws_handshake, this, cb, host,
                                     endpoint, std::placeholders::_1));
}

void stream::on_ws_handshake(stream::connect_handler cb, std::string host,
                             std::string endpoint,
                             const boost::system::error_code& ec)
{
  if (ec)
  {
#ifdef BINANCE_DEBUG
    std::cout << "websocket handshake error: " << ec << std::endl;
#endif
    cb(this, ec);
    return;
  }

#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
  connected_ = true;
#endif

  stream_->control_callback(
      boost::beast::bind_front_handler(&stream::on_control_frame, this));

  cb(this, {});
#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
  next_async_message();
#endif
}

void stream::on_control_frame(boost::beast::websocket::frame_type frame,
                              boost::string_view sv)
{
  boost::ignore_unused(sv);
  if (frame == boost::beast::websocket::frame_type::ping)
  {
    stream_->async_pong("", [](boost::system::error_code const& ec) {
      boost::ignore_unused(ec);
    });
  }
}

really_inline stream::operator bool() const
{
#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
  return stream_ && stream_->is_open() && connected_;
#else
  return stream_ && stream_->is_open();
#endif
}

void stream::subscribe(const std::vector<std::string>& topics)
{
  boost::json::object jv = {
      {"method", "SUBSCRIBE"}, {"params", topics}, {"id", id_++}};

#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
  messages_.push(boost::json::serialize(jv));
  next_async_message();
#else
  using namespace std::placeholders;
  stream_->async_write(boost::asio::buffer(boost::json::serialize(jv)),
                       std::bind(&stream::on_message_sent, this, _1, _2));
#endif
}

void stream::unsubscribe(const std::vector<std::string>& topics)
{
  boost::json::object jv = {
      {"method", "UNSUBSCRIBE"}, {"params", topics}, {"id", id_++}};

#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
  messages_.push(boost::json::serialize(jv));
  next_async_message();
#else
  using namespace std::placeholders;
  stream_->async_write(boost::asio::buffer(boost::json::serialize(jv)),
                       std::bind(&stream::on_message_sent, this, _1, _2));
#endif
}

#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
void stream::next_async_message()
{
  if (messages_.empty() || !connected_)
    return;

  auto& m = messages_.front();

  using namespace std::placeholders;
  stream_->async_write(boost::asio::buffer(m),
                       std::bind(&stream::on_message_sent, this, _1, _2));
}
#endif

void stream::on_message_sent(const boost::system::error_code& ec, size_t n)
{
  boost::ignore_unused(n);
  if (ec)
  {
#ifdef BINANCE_DEBUG
    std::cout << "error writing message: " << ec << std::endl;
#endif
    throw binance::error{ec};
  }

#ifdef BINANCE_WEBSOCKET_QUEUE_MESSAGES
  messages_.pop();
  next_async_message();
#endif
}
}  // namespace websocket
}  // namespace binance

#endif
