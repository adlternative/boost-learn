#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

std::string make_daytime_string() {
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
public:
  typedef boost::shared_ptr<tcp_connection> pointer;
  /* 创建一个实例 （通过create） 工厂模式*/
  static pointer create(boost::asio::io_context &io_context) {
    return pointer(new tcp_connection(io_context));
  }

  tcp::socket &socket() { return socket_; }

  void start() {
    message_ = make_daytime_string();
    /* 异步写 执行 async_write */
    /* 需要写的内容是 message_ */
    /* 回调函数是代表写完成之后做什么 */
    boost::asio::async_write(
        socket_, boost::asio::buffer(message_),
        boost::bind(&tcp_connection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  }

private:
  /* 创建一个socket */
  tcp_connection(boost::asio::io_context &io_context) : socket_(io_context) {}

  void handle_write(const boost::system::error_code & /*error*/,
                    size_t /*bytes_transferred*/) {}

  tcp::socket socket_;
  std::string message_;
};

class tcp_server {
public:
  tcp_server(boost::asio::io_context &io_context)
      : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), 13)) {
    start_accept();
  }

private:
  void start_accept() {
    std::cout << __func__ << std::this_thread::get_id() << std::endl;
    tcp_connection::pointer new_connection =
        tcp_connection::create(io_context_);
    /* 异步接受连接 */
    /* 异步执行handle_accept
     * 里面会让一个tcpConnection连接start,然后继续start_accept */
    acceptor_.async_accept(new_connection->socket(),
                           boost::bind(&tcp_server::handle_accept, this,
                                       new_connection,
                                       boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection,
                     const boost::system::error_code &error) {
    /* 在这之前boost帮我们做好了错误的设置和accept的工作 */
    std::cout << __func__ << std::this_thread::get_id() << std::endl;
    if (!error) {
      new_connection->start();
    }

    start_accept();
  }

  boost::asio::io_context &io_context_;
  tcp::acceptor acceptor_;
};

int main() {
  std::cout << __func__ << std::this_thread::get_id() << std::endl;

  try {
    boost::asio::io_context io_context;
    tcp_server server(io_context);
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}