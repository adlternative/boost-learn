#include <boost/asio.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/thread.hpp>
#include <iostream>
using namespace boost::asio;
int main(int argc, char const *argv[]) {
  io_service service;
  ip::tcp::endpoint ep;
  ip::tcp::socket sock(service);
  /* case0 */
  sock.connect(ep); // 第一行
  /* case1 */
  {
    boost::system::error_code err;
    sock.connect(ep, err); // 第二行
  }
  /* case2 */
  {
    try {
      sock.connect(ep);
    } catch (boost::system::system_error e) {
      std::cout << e.code() << std::endl;
    }
  }
  return 0;
}
