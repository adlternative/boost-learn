//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"
#include <boost/bind/bind.hpp>
#include <iostream>
#include <vector>

namespace http {
namespace server {

connection::connection(boost::asio::io_context &io_context,
                       connection_manager &manager, request_handler &handler)
    : socket_(io_context), connection_manager_(manager),
      request_handler_(handler) {}

boost::asio::ip::tcp::socket &connection::socket() { return socket_; }

void connection::start() {
  /* 套接字开始异步读（8092个字符） */
  socket_.async_read_some(
      boost::asio::buffer(buffer_),
      boost::bind(&connection::handle_read, shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}
/* 关闭连接也就是关闭套接字 */
void connection::stop() { socket_.close(); }

void connection::handle_read(const boost::system::error_code &e,
                             std::size_t bytes_transferred) {
  if (!e) {
    boost::tribool result;
    /* boost::tuples::ignore就是其他语言中的 _ */
    /* boost::tie可以接受一个元组 */
    /* buffer[b,e]->request_ */
    std::cout << bytes_transferred << std::endl;
    boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
        request_, buffer_.data(), buffer_.data() + bytes_transferred);
    /* 解析成功 */
    if (result) {
      /* 处理request的内容写到reply */
      request_handler_.handle_request(request_, reply_);
      /* 将reply异步发送 */
      boost::asio::async_write(socket_, reply_.to_buffers(),
                               boost::bind(&connection::handle_write,
                                           shared_from_this(),
                                           boost::asio::placeholders::error));
    } else if (!result) {
      /* 失败则返回一个400 */
      reply_ = reply::stock_reply(reply::bad_request);
      boost::asio::async_write(socket_, reply_.to_buffers(),
                               boost::bind(&connection::handle_write,
                                           shared_from_this(),
                                           boost::asio::placeholders::error));
    } else {
      /* 说明没读完继续异步读？ */
      // std::cout << "[TRACE]" << std::endl;
      socket_.async_read_some(
          boost::asio::buffer(buffer_),
          boost::bind(&connection::handle_read, shared_from_this(),
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred));
    }
  } else if (e != boost::asio::error::operation_aborted) {
    connection_manager_.stop(shared_from_this());
  }
}

void connection::handle_write(const boost::system::error_code &e) {
  if (!e) {
    /* 写完以后优雅关闭连接 */
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }
  /* 如果错误不是abort(信号)我们就用连接管理器关闭了它（内部socket.close() ) */
  if (e != boost::asio::error::operation_aborted) {
    connection_manager_.stop(shared_from_this());
  }
}

} // namespace server
} // namespace http