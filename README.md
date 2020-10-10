# Binance Futures C++ SDK
Binance Futures C++ SDK, unlike other C++ SDKs, works on top of Boost.Beast. No cURL,
no weird WebSocket libraries. Only Boost (and simdjson).

## Contents

* [Features](#Features)
* [HTTP(s) API](#http-api-client)
* [WebSocket](#websocket)
* [OS Signals](#handling-os-signals)
* [Build examples](#buid-examples)

## Features

* Request queuing.

  By default, Boost.ASIO doesn't support sending multiple requests at the same time.
  This library queues the request internally so you don't need to queue the request
  in your code.
  
* SIMD boosted.

  The library uses [simdjson](https://github.com/simdjson/simdjson) as json parser.
  Also, it does contain many other SIMD improvements like [string to int conversion](https://github.com/dgrr/binance-futures-sdk/blob/master/include/binance/conv.hpp#L15)
  and [decimal to hexadecimal conversion](https://github.com/dgrr/binance-futures-sdk/blob/master/include/binance/conv.hpp#L103) (the latter reuses the string buffer).

## HTTP API client

The client works only in ASYNC mode. That means that all your requests will be
queued internally and executed sequentially. After every execution, the library will call your
callback returning the response.

The client establishes a connection when you call the method `connect`.
The connection won't close unless you either destroy the object, call `close`
or the server closes the connection.
The connection is getting opened and closed around every 15 seconds.
This is due to some HTTP servers closes the connection if they don't get any request
in a given amount of time.
By keeping the connection alive we avoid any performance issues on
creating a connection for every HTTP request.

If the connection closed unexpectedly or you just want to reset
the connection, just call `connect` again.

## WebSocket

The WebSocket stream works only in ASYNC mode too unless for connecting.
All the messages sent are not queued, so you'll need to schedule the messages by your own.

If you get disconnected you can re-stablish the connection by calling `connect` again.
Remember, a websocket connection is only valid for 24h, so
every 24h you'll need to reconnect.

The WebSocket stream was built on usability with other services in mind.
For example, if you want to receive messages from Binance Futures and send the info
to an external service via WebSocket, you can by reusing the `io_context`.

Keep in mind that `stream` was built to run in a single-thread environment,
we do not know yet the consequences of running the `stream` in a multi-thread
context. (with Boost.ASIO/Boost.Beast should be easy to do it).

## Handling OS signals

Handling OS signals is up to you. With Boost.ASIO you can handle signals easily.

## Build examples
Usual building:
```bash
git clone https://github.com/dgrr/binance-futures-sdk
cd binance-futures-sdk
git submodule update --init --recursive
mkdir build
cd build
cmake -DBINANCE_DISABLE_THREADING:BOOL=ON -DBINANCE_USE_STRING_VIEW:BOOL=ON -DBINANCE_BUILD_EXAMPLES:BOOL=ON ..
cmake --build . --target all
```

Building with Docker:
```bash
git clone https://github.com/dgrr/binance-futures-sdk
cd binance-futures-sdk
git submodule update --init --recursive
./build.sh
```
