#include <boost/asio.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/thread.hpp>
#include <iostream>
using namespace boost::asio;
using namespace boost::placeholders;
typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;

void start_accept(socket_ptr sock);

io_service service;
ip::tcp::endpoint ep(ip::tcp::v4(), 2001); // 监听端口2001
ip::tcp::acceptor acc(service, ep);
socket_ptr sock(new ip::tcp::socket(service));
void signal_handler(const boost::system::error_code &err, int signal) {
  // 纪录日志，然后退出应用
}

int main(int argc, char const *argv[]) {
  boost::asio::signal_set sig(service, SIGINT, SIGTERM);
  sig.async_wait(signal_handler);

  start_accept(sock);
  service.run();
  return 0;
}
void handle_accept(socket_ptr sock, const boost::system::error_code &err) {
  if (err)
    return;
  // 从这里开始, 你可以从socket读取或者写入

  socket_ptr sock2(new ip::tcp::socket(service));
  start_accept(sock2);
}
void start_accept(socket_ptr sock) {
  acc.async_accept(*sock, boost::bind(handle_accept, sock, _1));
}