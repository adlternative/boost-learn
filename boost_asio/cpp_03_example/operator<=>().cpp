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
struct ID {
  int id_number;
  auto operator<=>(const ID &) const = default;
};

struct Person {
  ID id;
  string name;
  string email;
  std::weak_ordering operator<=>(const Person &other) const {
    return id <=> other.id;
  }
};
int main(int argc, char *argv[]) { return 0; }
