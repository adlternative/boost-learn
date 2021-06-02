#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

void print(const boost::system::error_code & /*e*/,
           boost::asio::steady_timer *t, int *count) {
  if (*count < 5) {
    std::cout << *count << std::endl;
    ++(*count);

    t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));
    /* 不会等待，这应该就是向全局的io_context
        注册了一个超时函数而已 */
    t->async_wait(
        boost::bind(print, boost::asio::placeholders::error, t, count));
  }
}

int main(int argc, char const *argv[]) {
  boost::asio::io_context io;
  int count = 0;

  boost::asio::steady_timer t(io, boost::asio::chrono::seconds(1));
  std::cout << "你好，世界1！" << std::endl;
  t.async_wait(
      boost::bind(print, boost::asio::placeholders::error, &t, &count));
  std::cout << "你好，世界2！" << std::endl;
  io.run(); /* 从这里开始跑print */
  std::cout << "Final count is " << count << std::endl;
  return 0;
}
