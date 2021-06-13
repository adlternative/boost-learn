#include <bits/stdc++.h> /* 万能头 */

using namespace std;
void copy(char *to, char *from, int count) {
  int n = (count + 7) / 8; /* (169+7)/8 == 22 */
  switch (count % 8) {     // 1
  case 0:
    *to = *from++;
  case 7:
    *to = *from++;

  case 6:
    *to = *from++;
  case 5:
    *to = *from++;
  case 4:
    *to = *from++;
  case 3:
    *to = *from++;
  case 2:
    *to = *from++;
  case 1:
    *to = *from++;
  }
  while (--n) { // 21*8=168
    *to = *from++;
    *to = *from++;
    *to = *from++;
    *to = *from++;
    *to = *from++;
    *to = *from++;
    *to = *from++;
    *to = *from++;
  }
}

void copy2(char *to, char *from, int count) {
  int n = (count + 7) / 8; /* (169+7)/8 == 22 */
  switch (count % 8) {     // 1
  case 0:
    do {
      *to = *from++;
    case 7:
      *to = *from++;
    case 6:
      *to = *from++;
    case 5:
      *to = *from++;
    case 4:
      *to = *from++;
    case 3:
      *to = *from++;
    case 2:
      *to = *from++;
    case 1:
      *to = *from++;
    } while (--n > 0);
  }
}
int main() {
  char *a = new char[170];
  char s[] = "abcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcab"
             "cabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabca"
             "bcabcabcabcabcabcabcabcabcabcabcabcabc";
  std::cout << strlen(s) << std::endl;
  copy2(a, s, 169);
  std::cout << s << std::endl;
  delete[] a;
}