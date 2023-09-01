#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <spinlock.h>

typedef struct {
    int         i_flags;
    spinlock_t  i_lock;
} inode_t;