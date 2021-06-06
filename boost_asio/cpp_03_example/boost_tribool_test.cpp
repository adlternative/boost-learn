#include <assert.h>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>
#include <iostream>
using namespace boost;
using namespace std;

int main() {
  tribool tb(true);
  tribool tb2(!tb);
  if (tb) {
    cout << "true" << endl;
  }
  tb2 = indeterminate;
  // tb2是不确定状态
  assert(indeterminate(tb2));
  //用indeterminate函数检测状态
  cout << tb2 << endl;
  if (tb2 == indeterminate) {
    cout << "indeterminate" << endl;
  }
  cout << (tb2 || true) << endl;
  cout << (tb2 && false) << endl;
  return 0;
}