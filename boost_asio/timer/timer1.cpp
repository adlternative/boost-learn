#include <boost/asio.hpp>
#include <iostream>

int main(int argc, char const *argv[]) {
  boost::asio::io_context io;
  boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
  t.wait();
  std::cout << "你好，世界！" << std::endl;
  return 0;
}
