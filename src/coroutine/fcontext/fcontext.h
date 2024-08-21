#ifndef FCONTEXT_H_
#define FCONTEXT_H_

#include <cstdint>

namespace zyweb {

typedef void *fcontext_t;

extern "C" intptr_t jump_fcontext(fcontext_t *ofc, fcontext_t nfc, intptr_t vp, bool preserve_fpu = false);
extern "C" fcontext_t make_fcontext(void *sp, std::size_t size, void (*fn)(intptr_t));

}

#endif

