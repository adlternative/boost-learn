//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "chat_message.hpp"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>

using boost::asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client {
public:
  chat_client(boost::asio::io_context &io_context,
              const tcp::resolver::results_type &endpoints)
      : io_context_(io_context), socket_(io_context) {
    boost::asio::async_connect(socket_, endpoints,
                               boost::bind(&chat_client::handle_connect, this,
                                           boost::asio::placeholders::error));
  }
  /* post()这个方法能立即返回，并且请求一个io_service实例调用制定的函数操作(function
   * handler)，之后会在某一个调用io_service.run()的线程中执行。 */
  void write(const chat_message &msg) {
    /* 异步 do_write 写 */
    boost::asio::post(io_context_,
                      boost::bind(&chat_client::do_write, this, msg));
  }
  /* 异步close */
  void close() {
    boost::asio::post(io_context_, boost::bind(&chat_client::do_close, this));
  }

private:
  /* 连上以后干什么 */
  void handle_connect(const boost::system::error_code &error) {
    if (!error) {
      /* 异步读 */
      boost::asio::async_read(
          socket_,
          /* 仅仅读header */
          boost::asio::buffer(read_msg_.data(), chat_message::header_length),
          boost::bind(&chat_client::handle_read_header, this,
                      boost::asio::placeholders::error));
    }
  }

  void handle_read_header(const boost::system::error_code &error) {
    if (!error && read_msg_.decode_header()) {
      /* header读完读body */
      boost::asio::async_read(
          socket_,
          boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
          boost::bind(&chat_client::handle_read_body, this,
                      boost::asio::placeholders::error));
    } else {
      do_close();
    }
  }

  void handle_read_body(const boost::system::error_code &error) {
    if (!error) {
      /* 读完body以后写body */
      std::cout.write(read_msg_.body(), read_msg_.body_length());
      std::cout << "\n";
      /* 写body以后读header */
      boost::asio::async_read(
          socket_,
          boost::asio::buffer(read_msg_.data(), chat_message::header_length),
          boost::bind(&chat_client::handle_read_header, this,
                      boost::asio::placeholders::error));
    } else {
      do_close();
    }
  }

  void do_write(chat_message msg) {
    bool write_in_progress = !write_msgs_.empty();
    /* 推入写队列中 */
    write_msgs_.push_back(msg);
    /* 如果现在写队列是空的 */
    if (!write_in_progress) {
      /* 执行异步写 */
      boost::asio::async_write(
          socket_,
          boost::asio::buffer(write_msgs_.front().data(),
                              write_msgs_.front().length()),
          boost::bind(&chat_client::handle_write, this,
                      boost::asio::placeholders::error));
    }
  }

  void handle_write(const boost::system::error_code &error) {
    if (!error) {
      write_msgs_.pop_front();
      /* 如果队列不是空的，再次执行异步写 */
      if (!write_msgs_.empty()) {
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(write_msgs_.front().data(),
                                write_msgs_.front().length()),
            boost::bind(&chat_client::handle_write, this,
                        boost::asio::placeholders::error));
      }
    } else {
      do_close();
    }
  }

  void do_close() { socket_.close(); }

private:
  boost::asio::io_context &io_context_;
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: chat_client <host> <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], argv[2]);

    chat_client c(io_context, endpoints);

    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));

    char line[chat_message::max_body_length + 1];
    while (std::cin.getline(line, chat_message::max_body_length + 1)) {
      using namespace std; // For strlen and memcpy.
      chat_message msg;
      msg.body_length(strlen(line));
      memcpy(msg.body(), line, msg.body_length());
      msg.encode_header();
      c.write(msg);
    }

    c.close();
    t.join();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
