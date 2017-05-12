#include "BTree.h"

int main() {
  BTree b;
  for (int i = 1; i <= 5; ++i) {
    b.Insert(i);
  }
  b.Delete(2);
  b.Delete(3);
  b.Delete(1);
  b.Display();
}