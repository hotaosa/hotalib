#include <iostream>
#include <string>

#include "hotaosa/ds/trie.h"

int main() {
  int q;
  std::cin >> q;
  hotaosa::Trie<26, 'a'> x, y;

  while (q--) {
    int t;
    std::string s;
    std::cin >> t >> s;
    if (t == 1) {
      y.RemoveWithPrefix(s);
      x.Insert(s);
    } else {
      if (!x.ContainsPrefixOf(s)) {
        y.Insert(s);
      }
    }
    std::cout << y.TotalCount() << "\n";
  }

  return 0;
}
