#ifndef _BASE_NONCOPYABLE_H
#define _BASE_NONCOPYABLE_H

namespace zyweb {

class NonCopyable {
 public:
    NonCopyable(const NonCopyable &) = delete;
    void operator=(const NonCopyable &) = delete;

 protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
};

}

#endif
