//
// Created by szy on 8/7/24.
//

#ifndef _TYPES_H_
#define _TYPES_H_

#include <string>

namespace zyweb {

using std::string;

template<typename To, typename From>
inline To implicit_cast(From const &f) {
  return f;
}

}

#endif
