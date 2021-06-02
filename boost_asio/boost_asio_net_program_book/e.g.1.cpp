#include <boost/asio.hpp>
#include <iostream>
/* g++ e.g.1.cpp  -lpthread */
using namespace boost::asio;
int main(int argc, char const *argv[]) {
  try {
    io_service service;
    ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 2001);
    ip::tcp::socket sock(service);
    sock.connect(ep);
  } catch (const boost::wrapexcept<boost::system::system_error> &e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "unknown bug" << std::endl;
  }
  return 0;
}
