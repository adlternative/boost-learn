#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

class printer {
public:
  /* 传入io上下文 用来注册定时器*/
  printer(boost::asio::io_context &io)
      : timer_(io, boost::asio::chrono::seconds(1)), count_(0) {
    timer_.async_wait(boost::bind(&printer::print, this));
  }

  ~printer() { std::cout << "Final count is " << count_ << std::endl; }

  void print() {
    if (count_ < 5) {
      std::cout << count_ << std::endl;
      ++count_;
      /* 设置超时在何时 */
      timer_.expires_at(timer_.expiry() + boost::asio::chrono::seconds(1));
      /* 卡死，说明真的是async_wait去注册的 */
      //       while (1)
      //         ;
      /* 真正注册开始 */
      timer_.async_wait(boost::bind(&printer::print, this));
      /* 会直接返回 */
      /* 超时时间一发生就会调用注册的print */
    }
  }

private:
  boost::asio::steady_timer timer_;
  int count_;
};

int main() {
  boost::asio::io_context io;
  /* 传入io上下文 */
  printer p(io);
  io.run();

  return 0;
}