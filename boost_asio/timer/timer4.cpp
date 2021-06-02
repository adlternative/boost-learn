#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#define BOOST_DATE_TIME_SOURCE

class printer {
public:
  printer(boost::asio::io_context &io)
      : strand_(boost::asio::make_strand(io)),
        /* 启动时间 */
        timer1_(io, boost::asio::chrono::seconds(1)),
        timer2_(io, boost::asio::chrono::seconds(1)), count_(0) {
    timer1_.async_wait(boost::asio::bind_executor(
        strand_, boost::bind(&printer::print1, this)));

    timer2_.async_wait(boost::asio::bind_executor(
        strand_, boost::bind(&printer::print2, this)));
  }
  ~printer() { std::cout << "Final count is " << count_ << std::endl; }
  void print1() {
    if (count_ < 10) {
      std::cout << "Timer 1: " << count_ << std::endl;
      ++count_;
      //       故意卡死
      //       while (1)
      //         ;
      timer1_.expires_at(timer1_.expiry() + boost::asio::chrono::seconds(1));
      /* 注意这个bind_executor将函数绑定到一个串行的执行器上，然后timer1异步返回
       */
      timer1_.async_wait(boost::asio::bind_executor(
          strand_, boost::bind(&printer::print1, this)));
    }
  }

  void print2() {
    /* 现在触发了print2 */
    //     std::cout << "现在触发了print2" << std::endl;
    if (count_ < 10) {
      auto begin = boost::posix_time::microsec_clock::universal_time();
      std::cout << "Timer 2: " << count_ << std::endl;
      ++count_;

      /* 间隔时间2 ，并非立即触发*/
      timer2_.expires_at(timer2_.expiry() + boost::asio::chrono::seconds(2));
      auto cur = boost::posix_time::microsec_clock::universal_time();
      auto time_elapse = cur - begin;
      std::cout << "过去了 " << time_elapse.total_microseconds() << std::endl;
      /* 异步调用 立刻返回 */
      timer2_.async_wait(boost::asio::bind_executor(
          strand_, boost::bind(&printer::print2, this)));
      cur = boost::posix_time::microsec_clock::universal_time();
      time_elapse = cur - begin;
      std::cout << "过去了 " << time_elapse.total_microseconds() << std::endl;
      /* 这里卡死试一试 (由于整体串行这里会全局卡死)*/
      //       while (1)
      //         ;
    }
  }

private:
  /* 用来串行化 */
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  boost::asio::steady_timer timer1_;
  boost::asio::steady_timer timer2_;
  int count_;
};
int main(int argc, char const *argv[]) {
  boost::asio::io_context io;
  int count = 0;
  printer p(io);
  boost::thread t(boost::bind(&boost::asio::io_context::run, &io));
  io.run();
  t.join();
  return 0;
}
