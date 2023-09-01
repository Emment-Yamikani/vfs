#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <spinlock.h>
#include <dentry.h>

typedef struct {
    char        *path;
    char        *fstype;
    int         flags;
    dentry_t    *mnt;
} vfsmount_t;