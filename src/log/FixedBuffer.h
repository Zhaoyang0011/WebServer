#ifndef _FIXED_BUFFER_H
#define _FIXED_BUFFER_H

#include <cstddef>
#include <cstring>
#include <string>
#include "src/base/NonCopyable.h"
namespace zyweb {

const int SMALL_BUFFER = 4000;
const int LARGE_BUFFER = 4000 * 1000;

template<int SIZE>
class FixedBuffer : NonCopyable {
 public:
    FixedBuffer()
        : _current(_data) {
        setCookie(cookieStart);
    }

    ~FixedBuffer() {
        setCookie(cookieEnd);
    }

    void append(const char * /*restrict*/ buf, size_t len) {
        // FIXME: append partially
        if (implicit_cast<size_t>(avail()) > len) {
            memcpy(_current, buf, len);
            _current += len;
        }
    }

    const char *getData() const { return _data; }
    int length() const { return static_cast<int>(_current - _data); }

    // write to data_ directly
    char *getCurrent() { return _current; }
    int avail() const { return static_cast<int>(end() - _current); }
    void add(size_t len) { _current += len; }

    void reset() { _current = _data; }
    void bzero() { memset(_data, 0, sizeof _data); }

    // for used by GDB
    const char *debugString();
    void setCookie(void (*cookie)()) { _cookie = cookie; }
    // for used by unit test

 private:
    const char *end() const { return _data + sizeof _data; }
    // Must be outline function for cookies.
    static void cookieStart();
    static void cookieEnd();

    void (*_cookie)();
    char _data[SIZE];
    char *_current;
};

}

#endif
