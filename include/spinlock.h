#pragma once

#include <sys/unistd.h>
#include <panic.h>
#include <stdatomic.h>
#include <pthread.h>

typedef struct
{
    int lock;
    pthread_t thread;
} spinlock_t;

#define SPINLOCK_INIT() ((spinlock_t){ \
    .lock = 0,                         \
    .thread = 0,                       \
})

#define spin_islocked(lk) ({                                \
    ((lk)->lock && (lk)->thread == pthread_self()) ? 1 : 0; \
})

#define spin_assert_locked(lk) ({                  \
    assert(spin_islocked(lk), "lock is not held"); \
})

#define spin_lock(lk) ({                                         \
    assert(!spin_islocked(lk), "lock is held");                  \
    while (__atomic_test_and_set(&(lk)->lock, __ATOMIC_SEQ_CST)) \
        asm __volatile__("pause");                               \
    (lk)->thread = pthread_self();                               \
})

#define spin_unlock(lk) ({                         \
    spin_assert_locked(lk);                        \
    (lk)->thread = 0;                              \
    __atomic_clear(&(lk)->lock, __ATOMIC_SEQ_CST); \
})
