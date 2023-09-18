#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <spinlock.h>
#include <dentry.h>

typedef struct {
    char        *path;
    char        *fstype;
    int         flags;
    void        *priv;
    dentry_t    *mnt;
} fs_mount_t;

#define MS_BIND     0x00000001
#define MS_REMOUNT  0x00000002
#define MS_MOVE     0x00000004

#define MS_SHARED   0x00000008
#define MS_SLAVE    0x00000010

#define MS_NODEV    0x00000020
#define MS_NOATIME  0x00000040
#define MS_NOEXEC   0x00000080
#define MS_RDONLY   0x00000100

int vfs_mount(const char *src, const char *dest, const char *type, unsigned long flags, const void *data);