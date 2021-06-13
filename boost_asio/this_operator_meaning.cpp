#include <bits/stdc++.h> /* 万能头 */

using namespace std;

class foo {
public:
  foo(foo &f) : mem(2 * f.mem) {}
  foo() : mem(3) {}
  void operator()(int a = 0, int b = 0) {
    std::cout << a << b << mem << std::endl;
  }
  void func() {
    mem = 2;
    foo (*this)(1, 2);
  }
  int mem;
};
int main(int argc, char *argv[]) {
  foo f;
  f.func();
}