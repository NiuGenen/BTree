#include "BTree.h"
#include <iostream>

int main() {
  BTree b(4);
  int i;
  for (i = 1; i <= 32; ++i) {
    b.Insert(i);
  }
  b.Display();
  char asd;
  while(1){
    b.Insert(i+1);
    std::cout << "----------" << i+1 << "------------" << std:: endl;
    b.Display();
    i += 1;
    std::cin >> asd;
  }
}
