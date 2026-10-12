/* ============================================================
 * cstack.h: A Small C Library (a stack of ints)
 * ============================================================
 * Written in plain C11 so it can be used from C, C++, or any
 * language with a C foreign function interface.
 *
 * Typical C library design, worth recognising:
 *   - an OPAQUE handle: callers only ever see "cstack*", the
 *     struct layout stays private to cstack.c
 *   - create/destroy pairs: the caller must remember to destroy
 *   - status codes instead of exceptions
 *   - out-parameters for results
 * ============================================================ */

#ifndef CSTACK_H
#define CSTACK_H

#include <stddef.h>   /* size_t: a C header, because C code includes this file too */

/* ------------------------------------------------------------
 * extern "C"
 * ------------------------------------------------------------
 * A C++ compiler "mangles" function names to encode parameter
 * types (that is how overloading works). A C compiler does not.
 * Without extern "C", C++ code including this header would look
 * for a mangled symbol such as _Z11cstack_pushP6cstacki and the
 * link would fail, because cstack.c exported plain "cstack_push".
 *
 * __cplusplus is only defined by C++ compilers, so the C compiler
 * never sees the extern "C" lines (C has no such syntax).
 * ------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct cstack cstack;   /* opaque: defined only in cstack.c */

typedef enum cstack_status {
    CSTACK_OK = 0,
    CSTACK_EMPTY = 1,
    CSTACK_NO_MEMORY = 2,
    CSTACK_NULL_ARGUMENT = 3
} cstack_status;

cstack* cstack_create(size_t initial_capacity);   /* returns NULL on failure */
void cstack_destroy(cstack* stack);               /* NULL is allowed, like free() */

cstack_status cstack_push(cstack* stack, int value);
cstack_status cstack_pop(cstack* stack, int* out_value);
size_t cstack_size(const cstack* stack);

const char* cstack_status_message(cstack_status status);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* CSTACK_H */
