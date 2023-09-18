#pragma once

#include <sys/unistd.h>
#include <panic.h>
#include <stdatomic.h>
#include <pthread.h>
#include <atomic.h>

typedef struct
{
    uint8_t     guard;
    uint8_t     locked;
    pthread_t   thread;
} spinlock_t;

#define SPINLOCK_INIT() ((spinlock_t){ \
    .guard = 0,                        \
    .locked = 0,                       \
    .thread = 0,                       \
})

#define spin_islocked(lk) ({                                      \
    while (__atomic_test_and_set(&(lk)->guard, __ATOMIC_SEQ_CST)) \
        asm __volatile__("pause");                                \
    int locked = ((lk)->locked &&                                 \
                  ((lk)->thread == pthread_self())) ? 1 : 0;      \
    __atomic_clear(&(lk)->guard, __ATOMIC_SEQ_CST);               \
    locked;                                                       \
})

#define spin_assert_locked(lk) ({                  \
    assert(spin_islocked(lk), "lock is not held"); \
})

#define spin_lock(lk) ({                                              \
    for (;;) {                                                        \
        while (__atomic_test_and_set(&(lk)->guard, __ATOMIC_SEQ_CST)) \
            asm __volatile__("pause");                                \
        assert(!(((lk)->locked) &&                                    \
                 ((lk)->thread == pthread_self())),                   \
               "lock already acquired");                              \
        if ((lk)->locked)                                             \
            __atomic_clear(&(lk)->guard, __ATOMIC_SEQ_CST);           \
        else break;                                                   \
    }                                                                 \
    (lk)->locked = 1;                                                 \
    (lk)->thread = pthread_self();                                    \
    __atomic_clear(&(lk)->guard, __ATOMIC_SEQ_CST);                   \
})

#define spin_unlock(lk) ({                                        \
    while (__atomic_test_and_set(&(lk)->guard, __ATOMIC_SEQ_CST)) \
        asm __volatile__("pause");                                \
    assert((((lk)->locked) &&                                     \
            ((lk)->thread == pthread_self())),                    \
           "lock not acquired");                                  \
    (lk)->locked = 0;                                             \
    (lk)->thread = 0;                                             \
    __atomic_clear(&(lk)->guard, __ATOMIC_SEQ_CST);               \
})
