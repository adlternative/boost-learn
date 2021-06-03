//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "chat_message.hpp"
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------
/* 建立的参加者抽象，这很好！（这样我们甚至就可以加机器人，群主，管理员...） */
class chat_participant {
public:
  virtual ~chat_participant() {}
  virtual void deliver(const chat_message &msg) = 0;
};

typedef boost::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room {
public:
  /* 聊天室添加一个参加者 */
  void join(chat_participant_ptr participant) {
    /* 参加者集合中插入它 */
    participants_.insert(participant);
    /* 所有的历史记录使用deliver传递给这个新的participant */
    std::for_each(recent_msgs_.begin(), recent_msgs_.end(),
                  boost::bind(&chat_participant::deliver, participant,
                              boost::placeholders::_1));
  }
  /* 离开就是在参加者集合中删除 */
  void leave(chat_participant_ptr participant) {
    participants_.erase(participant);
  }
  /* 将所有的消息传递给每一个”参加者“ */
  void deliver(const chat_message &msg) {
    /* 向聊天室的小写列表中添加消息 */
    recent_msgs_.push_back(msg);
    /* 这里竟然会排旧消息 */
    while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();
    /* 将所有的消息传递给每一个”参加者“ */
    std::for_each(participants_.begin(), participants_.end(),
                  boost::bind(&chat_participant::deliver,
                              boost::placeholders::_1, boost::ref(msg)));
  }

private:
  std::set<chat_participant_ptr> participants_;
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session : public chat_participant,
                     public boost::enable_shared_from_this<chat_session> {
public:
  chat_session(boost::asio::io_context &io_context, chat_room &room)
      : socket_(io_context), room_(room) {}

  tcp::socket &socket() { return socket_; }
  /* 开始一个会话 */
  void start() {
    /* 向聊天室中添加当前会话 */
    room_.join(shared_from_this());
    /* 异步读header（读客户端的消息） */
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        boost::bind(&chat_session::handle_read_header, shared_from_this(),
                    boost::asio::placeholders::error));
  }
  /* 会话的deliver就是将一个消息放到消息列表中然后异步发给客户端 */
  void deliver(const chat_message &msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      boost::asio::async_write(
          socket_,
          boost::asio::buffer(write_msgs_.front().data(),
                              write_msgs_.front().length()),
          boost::bind(&chat_session::handle_write, shared_from_this(),
                      boost::asio::placeholders::error));
    }
  }

  void handle_read_header(const boost::system::error_code &error) {
    if (!error && read_msg_.decode_header()) {
      /* 读完header读body */
      boost::asio::async_read(
          socket_,
          boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
          boost::bind(&chat_session::handle_read_body, shared_from_this(),
                      boost::asio::placeholders::error));
    } else {
      room_.leave(shared_from_this());
    }
  }

  void handle_read_body(const boost::system::error_code &error) {
    if (!error) {
      /* 读完body向聊天室发消息 */
      room_.deliver(read_msg_);
      boost::asio::async_read(
          socket_,
          boost::asio::buffer(read_msg_.data(), chat_message::header_length),
          boost::bind(&chat_session::handle_read_header, shared_from_this(),
                      boost::asio::placeholders::error));
    } else {
      room_.leave(shared_from_this());
    }
  }
  /* 如果消息列表还有消息就继续异步写 */
  void handle_write(const boost::system::error_code &error) {
    if (!error) {
      write_msgs_.pop_front();
      if (!write_msgs_.empty()) {
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(write_msgs_.front().data(),
                                write_msgs_.front().length()),
            boost::bind(&chat_session::handle_write, shared_from_this(),
                        boost::asio::placeholders::error));
      }
    } else {
      room_.leave(shared_from_this());
    }
  }

private:
  tcp::socket socket_;
  chat_room &room_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

typedef boost::shared_ptr<chat_session> chat_session_ptr;

//----------------------------------------------------------------------

class chat_server {
public:
  chat_server(boost::asio::io_context &io_context,
              const tcp::endpoint &endpoint)
      : io_context_(io_context), acceptor_(io_context, endpoint) {
    start_accept();
  }
  /* 接受连接 */
  void start_accept() {
    /* 新的会话（） */
    chat_session_ptr new_session(new chat_session(io_context_, room_));
    /* 异步连接会话 */
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&chat_server::handle_accept, this,
                                       new_session,
                                       boost::asio::placeholders::error));
  }

  void handle_accept(chat_session_ptr session,
                     const boost::system::error_code &error) {
    if (!error) {
      /* 开始会话 */
      session->start();
    }
    /* 继续接受新的连接 */
    start_accept();
  }

private:
  boost::asio::io_context &io_context_;
  tcp::acceptor acceptor_;
  chat_room room_;
};

typedef boost::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;

//----------------------------------------------------------------------

int main(int argc, char *argv[]) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_context io_context;

    chat_server_list servers;
    for (int i = 1; i < argc; ++i) {
      /* 开多个不同的端口 */
      using namespace std; // For atoi.
      tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
      chat_server_ptr server(new chat_server(io_context, endpoint));
      /* 服务器列表 */
      servers.push_back(server);
    }

    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}