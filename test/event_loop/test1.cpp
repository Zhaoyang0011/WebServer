#include "EventLoop.h"

int main() {
  int &&a = 10;
  a = 100;
  int *p = &a;
  *p = 1;
}