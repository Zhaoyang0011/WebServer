#include <execinfo.h>
#include <cxxabi.h>
#include "CurrentThread.h"
#include "base/Timestamp.h"

namespace zyweb::CurrentThread {

__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char *t_threadName = "unknown";

void cacheTid() {
  if (t_cachedTid == 0) {
    t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

bool isMainThread() {
  return tid() == ::getpid();
}

string stackTrace(bool demangle) {
  string stack;
  const int max_frames = 200;
  void *frame[max_frames];
  int nptrs = ::backtrace(frame, max_frames);
  char **strings = ::backtrace_symbols(frame, nptrs);
  if (strings) {
    size_t len = 256;
    char *demangled = demangle ? static_cast<char *>(::malloc(len)) : nullptr;
    for (int i = 1; i < nptrs; ++i)  // skipping the 0-th, which is this function
    {
      if (demangle) {
        // https://panthema.net/2008/0901-stacktrace-demangled/
        // bin/exception_test(_ZN3Bar4testEv+0x79) [0x401909]
        char *left_par = nullptr;
        char *plus = nullptr;
        for (char *p = strings[i]; *p; ++p) {
          if (*p == '(')
            left_par = p;
          else if (*p == '+')
            plus = p;
        }

        if (left_par && plus) {
          *plus = '\0';
          int status = 0;
          char *ret = abi::__cxa_demangle(left_par + 1, demangled, &len, &status);
          *plus = '+';
          if (status == 0) {
            demangled = ret;  // ret could be realloc()
            stack.append(strings[i], left_par + 1);
            stack.append(demangled);
            stack.append(plus);
            stack.push_back('\n');
            continue;
          }
        }
      }
      // Fallback to mangled names
      stack.append(strings[i]);
      stack.push_back('\n');
    }
    free(demangled);
    free(strings);
  }
  return stack;
}

void sleepUsec(int64_t usec) {
  struct timespec ts = {0, 0};
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, nullptr);
}

}

