#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::results_type endpoints =
        resolver.resolve(argv[1], "daytime");
    //     boost::asio::ip::tcp::endpoint endpoints(
    //         boost::asio::ip::address::from_string("127.0.0.1"), 8080); 传
    /* 创建客户端套接字 */
    boost::asio::ip::tcp::socket socket(io_context);
    /* 创建与服务器的链接 */
    //     boost::asio::connect(socket, &endpoints);
    boost::asio::connect(socket, endpoints);
    for (;;) {
      boost::array<char, 128> buf;
      boost::system::error_code error;
      /* boost::asio::buffer() 函数自动确定数组的大小以帮助防止缓冲区溢出。 */
      size_t len = socket.read_some(boost::asio::buffer(buf), error);
      /* 连接被对等端干净地关闭。*/
      if (error == boost::asio::error::eof) {
        std::cout << "我关了" << std::endl;
        break;
      }               // Connection closed cleanly by peer.
      else if (error) // 其他一些错误。
        throw boost::system::system_error(error); // Some other error.

      std::cout.write(buf.data(), len);
    }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}