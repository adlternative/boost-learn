//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/aligned_storage.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <cstdlib>
#include <iostream>

using boost::asio::ip::tcp;

// Class to manage the memory to be used for handler-based custom allocation.
// It contains a single block of memory which may be returned for allocation
// requests. If the memory is in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
class handler_memory : private boost::noncopyable {
public:
  handler_memory() : in_use_(false) {}

  void *allocate(std::size_t size) {
    /* 如果没有使用并且请求大小比对齐存储块的大小小 */
    if (!in_use_ && size < storage_.size) {
      in_use_ = true;
      /* 返回存储块的地址 */
      return storage_.address();
    } else {
      /* 否则new分配 */
      return ::operator new(size);
    }
  }

  void deallocate(void *pointer) {
    if (pointer == storage_.address()) {
      in_use_ = false;
    } else {
      ::operator delete(pointer);
    }
  }

private:
  // Storage space used for handler-based custom memory allocation.
  boost::aligned_storage<1024> storage_;

  // Whether the handler-based custom allocation storage has been used.
  bool in_use_;
};

// The allocator to be associated with the handler objects. This allocator only
// needs to satisfy the C++11 minimal allocator requirements, plus rebind when
// targeting C++03.
template <typename T> class handler_allocator {
public:
  typedef T value_type;

  explicit handler_allocator(handler_memory &mem) : memory_(mem) {}

  template <typename U>
  handler_allocator(const handler_allocator<U> &other)
      : memory_(other.memory_) {}

  template <typename U> struct rebind { typedef handler_allocator<U> other; };

  bool operator==(const handler_allocator &other) const {
    return &memory_ == &other.memory_;
  }

  bool operator!=(const handler_allocator &other) const {
    return &memory_ != &other.memory_;
  }
  /* 分配n块T大小的内存 */
  T *allocate(std::size_t n) const {
    return static_cast<T *>(memory_.allocate(sizeof(T) * n));
  }
  /* 反正分配的虚拟内存连续,直接delete归还即可 */
  void deallocate(T *p, std::size_t /*n*/) const {
    return memory_.deallocate(p);
  }

  // private:
  // The underlying memory.
  handler_memory &memory_;
};

// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. The allocator_type typedef and get_allocator()
// member function are used by the asynchronous operations to obtain the
// allocator. Calls to operator() are forwarded to the encapsulated handler.
template <typename Handler> class custom_alloc_handler {
public:
  /* 让人摸不照头脑的Handler类型（嗯？这里不是一个bind么？）或者我们可以这么理解
  bind返回的是一个函数对象，里面的对象都是拷贝的...所以这里我们需要为他们分配内存？
*/
  typedef handler_allocator<Handler> allocator_type;

  custom_alloc_handler(handler_memory &m, Handler h)
      : memory_(m), handler_(h) {}
  /* 返回一块handler_allocator<Handler> */
  allocator_type get_allocator() const { return allocator_type(memory_); }

  template <typename Arg1> void operator()(Arg1 arg1) { handler_(arg1); }

  template <typename Arg1, typename Arg2>
  void operator()(Arg1 arg1, Arg2 arg2) {
    handler_(arg1, arg2);
  }

private:
  handler_memory &memory_;
  Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline custom_alloc_handler<Handler>
make_custom_alloc_handler(handler_memory &m, Handler h) {
  return custom_alloc_handler<Handler>(m, h);
}

class session : public boost::enable_shared_from_this<session> {
public:
  session(boost::asio::io_context &io_context) : socket_(io_context) {}

  tcp::socket &socket() { return socket_; }

  void start() {
    /* 异步读取
    make_custom_alloc_handler里面返回一个custom_alloc_handler函数对象 m =
    handler_memory_, h = boost::bind(&session::handle_read....
    再然后回调会是这个函数对象 custom_alloc_handler() ·*/
    socket_.async_read_some(
        boost::asio::buffer(data_),
        make_custom_alloc_handler(
            handler_memory_, /* m */
            boost::bind(&session::handle_read, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred) /* h */));
  }

  void handle_read(const boost::system::error_code &error,
                   size_t bytes_transferred) {
    if (!error) {
      boost::asio::async_write(
          socket_, boost::asio::buffer(data_, bytes_transferred),
          make_custom_alloc_handler(
              handler_memory_,
              boost::bind(&session::handle_write, shared_from_this(),
                          boost::asio::placeholders::error)));
    }
  }

  void handle_write(const boost::system::error_code &error) {
    if (!error) {
      socket_.async_read_some(
          boost::asio::buffer(data_),
          make_custom_alloc_handler(
              handler_memory_,
              boost::bind(&session::handle_read, shared_from_this(),
                          boost::asio::placeholders::error,
                          boost::asio::placeholders::bytes_transferred)));
    }
  }

private:
  // The socket used to communicate with the client.
  tcp::socket socket_;

  // Buffer used to store data received from the client.
  boost::array<char, 1024> data_;

  // The memory to use for handler-based custom memory allocation.
  /* 内存分配器 */
  handler_memory handler_memory_;
};

typedef boost::shared_ptr<session> session_ptr;

class server {
public:
  server(boost::asio::io_context &io_context, short port)
      : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    /* 创建会话 */
    session_ptr new_session(new session(io_context_));
    /* 异步连接，连接后执行 handle_accept */
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this,
                                       new_session,
                                       boost::asio::placeholders::error));
  }

  void handle_accept(session_ptr new_session,
                     const boost::system::error_code &error) {
    /* 开启会话 */
    if (!error) {
      new_session->start();
    }
    /* 删除了旧的会话 创建新的会话*/
    new_session.reset(new session(io_context_));
    /* 继续异步接受连接 */
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept, this,
                                       new_session,
                                       boost::asio::placeholders::error));
  }

private:
  boost::asio::io_context &io_context_;
  tcp::acceptor acceptor_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    using namespace std; // For atoi.
    server s(io_context, atoi(argv[1]));

    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}