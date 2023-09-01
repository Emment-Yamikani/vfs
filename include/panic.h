#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>

#define panic(...) ({    \
    printf(__VA_ARGS__); \
    exit(-1);            \
})

#define assert_msg(condition, ...) ({ \
    if (!(condition))                 \
        panic(__VA_ARGS__);           \
})

#define assert(condition, __msg) ({                          \
    assert_msg((condition), "%s:%d: thread[%ld] %s.\n",          \
               __FILE__, __LINE__, pthread_self(), (__msg)); \
})
