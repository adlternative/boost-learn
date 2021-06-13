#include <iostream> /*输入输出*/

#include <sys/types.h>/*  一堆类型*/
#include <sys/wait.h> /* 进程 wait */

#include <fcntl.h>   /* 文件操作 */
#include <sys/stat.h>/* 文件 stat */
#include <unistd.h>  /* 文件操作 */

#include <memory>/* 智能指针 */

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
/* stl */
#include <map>
#include <string>
#include <vector>
/* web */
#include <netinet/in.h>
#include <sys/socket.h>
using namespace std;

class Temp {
public:
  Temp(int &v) : value(v) {}
  operator int() const { return value; }
  int &operator=(int v) { return value = v; }
  ~Temp() { value = -1; }
  int &value;
};

int main(int argc, char *argv[]) {

  int a = 0;
  switch (Temp t = a)
  case 0:
    if (0) {
      std::cout << __LINE__ << " " << t << std::endl;
      std::cout << __LINE__ << " " << a << std::endl;
    } else
    case 1: {
      std::cout << __LINE__ << " " << t << std::endl;
      std::cout << __LINE__ << " " << a << std::endl;
    }
      std::cout << __LINE__ << " " << a << std::endl;

  return 0;
}