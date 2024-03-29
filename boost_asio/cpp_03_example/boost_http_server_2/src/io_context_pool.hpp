//
// io_context_pool.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER2_IO_SERVICE_POOL_HPP
#define HTTP_SERVER2_IO_SERVICE_POOL_HPP

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <vector>

namespace http {
namespace server2 {

/// A pool of io_context objects.
class io_context_pool : private boost::noncopyable {
public:
  /// Construct the io_context pool.
  explicit io_context_pool(std::size_t pool_size);

  /// Run all io_context objects in the pool.
  void run();

  /// Stop all io_context objects in the pool.
  void stop();

  /// Get an io_context to use.
  boost::asio::io_context &get_io_context();

private:
  typedef boost::shared_ptr<boost::asio::io_context> io_context_ptr;

  /// The pool of io_contexts.
  /* io上下文池 */
  std::vector<io_context_ptr> io_contexts_;

  /// The work-tracking executors that keep the io_contexts running.
  /* 许多用来让io上下文跑的工作追踪执行器 */
  std::list<boost::asio::any_io_executor> work_;

  /// The next io_context to use for a connection.
  std::size_t next_io_context_;
};

} // namespace server2
} // namespace http

#endif // HTTP_SERVER2_IO_SERVICE_POOL_HPP