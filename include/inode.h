#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <spinlock.h>

struct iops;

typedef enum {
    FS_INV,
    FS_REG,
    FS_DIR,
    FS_CHR,
    FS_FIFO,
    FS_SYM,
    FS_BLK,
} itype_t;

typedef struct {
    itype_t     i_type;
    int         i_flags;
    ssize_t     i_count;
    ssize_t     i_links;     
    struct iops *i_ops;
    void        *i_priv;
    
    spinlock_t  i_lock;
} inode_t;

typedef struct iops {
    int     (*link)();
    int     (*close)();
    ssize_t (*read)(inode_t *ip, __off_t off, void *buf, size_t nb);
    ssize_t (*write)(inode_t *ip, __off_t off, void *buf, size_t nb);
    int     (*mknod)();
    int     (*fnctl)();
    int     (*ioctl)();
    int     (*mkdir)();
    int     (*unlink)();
    int     (*lookup)();
    int     (*create)();
    int     (*rename)();
    int     (*readdir)();
    int     (*symlink)();
    int     (*getattr)();
    int     (*setattr)();
    int     (*truncate)();
} iops_t;

#define iassert(ip)         ({ assert((ip), "No inode"); })
#define ilock(ip)           ({ iassert(ip); spin_lock(&(ip)->i_lock); })
#define iunlock(ip)         ({ iassert(ip); spin_unlock(&(ip)->i_lock); })
#define ilocked(ip)         ({ iassert(ip); spin_islocked(&(ip)->i_lock); })
#define iassert_locked(ip)  ({ iassert(ip); spin_assert_locked(&(ip)->i_lock); })

int ialloc(inode_t **pip);



int     ilink(inode_t *ip);
int     iclose(inode_t *ip);
ssize_t iread(inode_t *ip, __off_t off, void *buf, size_t nb);
ssize_t iwrite(inode_t *ip, __off_t off, void *buf, size_t nb);
int     imknod(inode_t *ip);
int     ifnctl(inode_t *ip);
int     iioctl(inode_t *ip);
int     imkdir(inode_t *ip);
int     iunlink(inode_t *ip);
int     ilookup(inode_t *ip);
int     icreate(inode_t *ip);
int     irename(inode_t *ip);
int     ireaddir(inode_t *ip);
int     isymlink(inode_t *ip);
int     igetattr(inode_t *ip);
int     isetattr(inode_t *ip);
int     itruncate(inode_t *ip);