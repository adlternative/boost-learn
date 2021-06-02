#include <boost/asio.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/thread.hpp>
#include <iostream>
/* 不可编译 */
using namespace boost::asio;
bool reads = false;

void deadline_handler(const boost::system::error_code &) {
  std::cout << (((reads) ? "reads successfully" : "reads failed")) << std::endl;
}
void reads_handler(const boost::system::error_code &) { reads = true; }

int main(int argc, char const *argv[]) {
  io_service service;
  ip::tcp::socket sock(service);
  reads = false;
  char data[512];
  boost::system::error_code ec;
  sock.async_read_some(buffer(data, 512), reads_handler);
  deadline_timer t(service, boost::posix_time::milliseconds(100));
  t.async_wait(&deadline_handler);
  service.run();
  return 0;
}
