#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <spinlock.h>
#include <panic.h>

#define DCACHE_MOUNTED      1
#define DCACHE_CAN_FREE     2
#define DCACHE_REFERENCED   4


typedef struct dentry {
    char            *d_name;
    long            d_refs;
    long            d_flags;
    struct dentry   *d_next;
    struct dentry   *d_prev;
    struct dentry   *d_child;
    struct dentry   *d_parent;
    spinlock_t      d_lock;
} dentry_t;

#define dassert(dentry) ({                \
    assert((dentry), "No dentry struct"); \
})

#define dassert_locked(dentry) ({          \
    dassert(dentry);                       \
    spin_assert_locked(&(dentry)->d_lock); \
})

#define dlocked(dentry) ({            \
    spin_islocked(&(dentry)->d_lock); \
})

#define dlock(dentry) ({          \
    dassert(dentry);              \
    spin_lock(&(dentry)->d_lock); \
})

#define dunlock(dentry) ({          \
    dassert(dentry);                \
    spin_unlock(&(dentry)->d_lock); \
})

#define dsetflags(dentry, flags) ({\
    (dentry)->d_flags |= (flags);\
})

#define dunsetflags(dentry, flags) ({\
    (dentry)->d_flags &= ~(flags);\
})


void ddump(dentry_t *dentry, int flags);
#define DDUMP_HANG  1
#define DDUMP_PANIC 2

void ddup(dentry_t *dentry);
void dclose(dentry_t *dentry);
void dunbind(dentry_t *dentry);
void drelease(dentry_t *dentry);
int dbind(dentry_t *parent, dentry_t *child);
int dalloc(const char *name, dentry_t **pdentry);
int dlookup(dentry_t *dir, const char *name, dentry_t **pchild);