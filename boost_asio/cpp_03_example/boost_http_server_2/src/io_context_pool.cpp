//
// io_context_pool.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <stdexcept>

namespace http {
namespace server2 {

io_context_pool::io_context_pool(std::size_t pool_size) : next_io_context_(0) {
  if (pool_size == 0)
    throw std::runtime_error("io_context_pool size is 0");

  // Give all the io_contexts work to do so that their run() functions will not
  // exit until they are explicitly stopped.
  for (std::size_t i = 0; i < pool_size; ++i) {
    io_context_ptr io_context(new boost::asio::io_context);
    /* io上下文列表推新的io上下文 */
    io_contexts_.push_back(io_context);
    /* 似乎是向 any_io_executor执行其链表中添加了新的io上下文的执行器 */
    work_.push_back(
        boost::asio::require(io_context->get_executor(),
                             boost::asio::execution::outstanding_work.tracked));
  }
}

void io_context_pool::run() {
  /* 线程是run的时候创建的 */
  // Create a pool of threads to run all of the io_contexts.
  std::vector<boost::shared_ptr<boost::thread>> threads;
  /* 大家一起run起来就好 */
  for (std::size_t i = 0; i < io_contexts_.size(); ++i) {
    boost::shared_ptr<boost::thread> thread(new boost::thread(
        boost::bind(&boost::asio::io_context::run, io_contexts_[i])));
    threads.push_back(thread);
  }
  /* 主线程义不容辞等大家死 */
  // Wait for all threads in the pool to exit.
  for (std::size_t i = 0; i < threads.size(); ++i)
    threads[i]->join();
}

void io_context_pool::stop() {
  // Explicitly stop all io_contexts.
  for (std::size_t i = 0; i < io_contexts_.size(); ++i)
    io_contexts_[i]->stop();
}

/* 获得下一个io_context */
boost::asio::io_context &io_context_pool::get_io_context() {
  // Use a round-robin scheme to choose the next io_context to use.
  /* 典型轮询 */
  boost::asio::io_context &io_context = *io_contexts_[next_io_context_];
  ++next_io_context_;
  if (next_io_context_ == io_contexts_.size())
    next_io_context_ = 0;
  return io_context;
}

} // namespace server2
} // namespace http