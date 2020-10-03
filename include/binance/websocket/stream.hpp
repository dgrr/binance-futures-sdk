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
  uint64_t id_;

public:
  stream()               = delete;
  stream(const stream&)  = delete;
  stream(const stream&&) = delete;
  stream(binance::io_context& ioc);
  ~stream();

  uint64_t id() const;
  // closes the connection gracefully.
  really_inline void close();
  // connect creates a new connection
  //
  // if you call connect with the connection open,
  // the current connection will close.
  void connect();
  // connect creates a new connection and subscribes
  // the websocket connection to the user data streams
  // using the listen_key.
  void connect(const std::string& listen_key);
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
  void connect(const std::string& endpoint,
               boost::asio::ssl::verify_mode v_mode);
  void on_control_frame(boost::beast::websocket::frame_type,
                        boost::string_view);
  really_inline void reset();
};

stream::stream(binance::io_context& ioc)
    : ioc_(ioc)
    , ctx_(boost::asio::ssl::context::tlsv12_client)
    , id_(1)
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

template<typename... Topic>
void stream::subscribe(Topic... topics)
{
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
  // stream_->close(boost::beast::websocket::normal, ec);
  // stream_->next_layer().shutdown(ec);
  stream_->next_layer().next_layer().close();
}

void stream::reset()
{
  if (stream_)
    // gracefully close the connection
    close();
  stream_.emplace(ioc_, ctx_);
}

void stream::connect()
{
  connect("/ws/", boost::asio::ssl::verify_none);
}

void stream::connect(const std::string& listen_key)
{
  connect("/ws/" + listen_key, boost::asio::ssl::verify_none);
}

void stream::connect(const std::string& endpoint,
                     boost::asio::ssl::verify_mode v_mode)
{
  reset();

  namespace beast     = boost::beast;      // from <boost/beast.hpp>
  namespace http      = beast::http;       // from <boost/beast/http.hpp>
  namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
  namespace net       = boost::asio;       // from <boost/asio.hpp>
  namespace ssl       = boost::asio::ssl;  // from <boost/asio/ssl.hpp>
  using tcp           = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

  tcp::resolver resolver{ioc_};
  std::string host = BINANCE_WS_HOST;
  std::string port = "443";

  boost::system::error_code ec;
  auto const results = resolver.resolve(host, port, ec);
  if (ec)
  {
#ifdef BINANCE_DEBUG
    std::cout << "resolve error: " << ec << std::endl;
#endif
    throw binance::error{ec};
  }

  net::connect(beast::get_lowest_layer(*stream_), results.begin(),
               results.end(), ec);
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
    throw binance::error{beast::error_code{static_cast<int>(::ERR_get_error()),
                                           net::error::get_ssl_category()}};
  }

  stream_->next_layer().handshake(ssl::stream_base::client, ec);
  if (ec)
  {
#ifdef BINANCE_DEBUG
    std::cout << "SSL handshake error: " << ec << std::endl;
#endif
    throw binance::error{ec};
  }

  // if (!opts.disable_compression)
  // {
  //   websocket::permessage_deflate msg_def;
  //   msg_def.client_enable = true;
  //   stream_->set_option(msg_def);
  // }
  stream_->set_option(
      websocket::stream_base::decorator([](websocket::request_type& req) {
        req.set(http::field::user_agent, BINANCE_VERSION_STRING);
      }));

  stream_->handshake(host, endpoint, ec);
  stream_->control_callback(
      boost::beast::bind_front_handler(&stream::on_control_frame, this));
  if (ec)
  {
#ifdef BINANCE_DEBUG
    std::cout << "websocket handshake error: " << ec << std::endl;
#endif
    throw binance::error{ec};
  }
}

void stream::on_control_frame(boost::beast::websocket::frame_type frame,
                              boost::string_view sv)
{
  boost::ignore_unused(sv);
  if (frame == boost::beast::websocket::frame_type::ping)
  {
    stream_->async_pong(nullptr, [](boost::system::error_code const& ec) {
      boost::ignore_unused(ec);
    });
  }
}

really_inline stream::operator bool() const
{
  return stream_ && stream_->is_open();
}

void stream::subscribe(const std::vector<std::string>& topics)
{
  boost::json::object jv = {
      {"method", "SUBSCRIBE"}, {"params", topics}, {"id", id_++}};

  stream_->async_write(boost::asio::buffer(boost::json::serialize(jv)),
                       [](boost::system::error_code const& ec, std::size_t n) {
                         boost::ignore_unused(n);
                         if (ec)
                           throw ec;
                       });
}

void stream::unsubscribe(const std::vector<std::string>& topics)
{
  boost::json::object jv = {
      {"method", "UNSUBSCRIBE"}, {"params", topics}, {"id", id_++}};

  stream_->async_write(boost::asio::buffer(boost::json::serialize(jv)),
                       [](boost::system::error_code const& ec, std::size_t n) {
                         boost::ignore_unused(n);
                         if (ec)
                           throw ec;
                       });
}
}  // namespace websocket
}  // namespace binance

#endif