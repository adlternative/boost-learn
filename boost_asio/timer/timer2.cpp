#include <boost/asio.hpp>
#include <iostream>
/* 共享的资源 std::cout */
void print(const boost::system::error_code & /*e*/) {
  std::cout << "Hello, world!" << std::endl;
}
int main(int argc, char const *argv[]) {
  boost::asio::io_context io;
  boost::asio::steady_timer t(io, boost::asio::chrono::seconds(2));
  boost::asio::steady_timer t2(io, boost::asio::chrono::seconds(3));
  std::cout << "你好，世界3！" << std::endl;
  /* 直接返回 */
  t.async_wait(&print);
  t2.async_wait(&print);
  std::cout << "你好，世界2！" << std::endl;
  io.run();
  /* 后面当然会阻塞直到io.run 结束 */
  std::cout << "你好，世界4！" << std::endl;
  return 0;
}
