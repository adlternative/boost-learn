//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include <boost/bind/bind.hpp>
#include <signal.h>

namespace http {
namespace server {

server::server(const std::string &address, const std::string &port,
               const std::string &doc_root)
    : io_context_(), signals_(io_context_), acceptor_(io_context_),
      connection_manager_(), new_connection_(), request_handler_(doc_root) {
  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through Asio.
  /* 信号注册 */
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

  /* 遇到信号则回调handle_stop */
  signals_.async_wait(boost::bind(&server::handle_stop, this));

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  boost::asio::ip::tcp::resolver resolver(io_context_);
  /* reslove　后应该就是它绑定的地址 */
  boost::asio::ip::tcp::endpoint endpoint =
      *resolver.resolve(address, port).begin();
  /* 似乎是这里以endpoint的指定协议打开（v4/v6） */
  acceptor_.open(endpoint.protocol());
  /* SO_REUSEADDR */
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();
  /* 开始接受连接 */
  start_accept();
}

void server::run() {
  // The io_context::run() call will block until all asynchronous operations
  // have finished. While the server is running, there is always at least one
  // asynchronous operation outstanding: the asynchronous accept call waiting
  // for new incoming connections.
  io_context_.run();
}

void server::start_accept() {
  /* 开始连接的时候*/
  /* 注意这个new_connection_是个shared_ptr，我们下一次start_accept的时候
   只是引用计数-1*/
  /* The next connection to be accepted. */
  new_connection_.reset(
      new connection(io_context_, connection_manager_, request_handler_));
  acceptor_.async_accept(new_connection_->socket(),
                         boost::bind(&server::handle_accept, this,
                                     boost::asio::placeholders::error));
}

void server::handle_accept(const boost::system::error_code &e) {
  // Check whether the server was stopped by a signal before this completion
  // handler had a chance to run.
  /* 如果我们在异步执行accept完成之后server已经执行了handle_stop
                大概是这个流程： start_accept->handle_stop->handle_accept
 */
  if (!acceptor_.is_open()) {
    return;
  }

  if (!e) {
    /* 连接管理器开始工作 */
    connection_manager_.start(new_connection_);
  }
  /* 继续去异步accept */
  start_accept();
}

void server::handle_stop() {
  // The server is stopped by cancelling all outstanding asynchronous
  // operations. Once all operations have finished the io_context::run() call
  // will exit.
  /* 如果我们在遇到信号的时候 先将acceptor关了*/
  acceptor_.close();
  connection_manager_.stop_all();
}

} // namespace server
} // namespace http
