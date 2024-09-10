#include <iostream>
#include <list>
#include "coroutine/Coroutine.h"

using namespace zyweb;

void runInCoroutine() {
  std::cout << "Step1";
  Coroutine::YieldToReady();
  std::cout << "Step2";
  Coroutine::YieldToReady();
  std::cout << "Step3";
  Coroutine::YieldToReady();
  std::cout << "Step4";
  Coroutine::YieldToReady();
  std::cout << "Step5";
  Coroutine::YieldToReady();
}

int main() {
  Coroutine::GetThis();
  std::shared_ptr<Coroutine> coroutine(Coroutine::NewCoroutine(runInCoroutine, 0, false));
  coroutine->swapIn();
  coroutine->swapIn();
  coroutine->swapIn();
  coroutine->swapIn();
  coroutine->swapIn();
  return 0;
}