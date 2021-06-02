#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>
using namespace boost::asio;
typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
/* cd "/home/adl/boost_learn/" && g++ e.g.3.cpp -g -lpthread -o e.g.3 && "/home/adl/boost_learn/"e.g.3 */
void connect_handler(const boost::system::error_code &ec) {
  // 如果ec返回成功我们就可以知道连接成功了
  if (ec)
    std::cout << "失败" << std::endl;
  else
    std::cout << "成功" << std::endl;
}

int main(int argc, char const *argv[]) {

  io_service service;
  ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 2001);
  ip::tcp::socket sock(service);
  sock.async_connect(ep, connect_handler);
  service.run();
  return 0;
}
