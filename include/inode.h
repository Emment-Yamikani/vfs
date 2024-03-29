#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <spinlock.h>
#include <sys/types.h>

struct iops;
struct dentry;

typedef enum {
    FS_INV,
    FS_REG,
    FS_DIR,
    FS_CHR,
    FS_SYM,
    FS_BLK,
    FS_FIFO,
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
    int     (*ilink)(struct dentry *oldname, inode_t *dir, struct dentry *newname);
    int     (*ibind)(inode_t *dir, struct dentry *dentry, inode_t *ip);
    int     (*irmdir)(inode_t *ip);
    int     (*isync)(inode_t *ip);
    int     (*iclose)(inode_t *ip);
    int     (*iunlink)(inode_t *ip);
    int     (*itruncate)(inode_t *ip);
    ssize_t (*iread)(inode_t *ip, __off_t off, void *buf, size_t nb);
    ssize_t (*iwrite)(inode_t *ip, __off_t off, void *buf, size_t nb);
    int     (*imknod)(inode_t *dir, struct dentry *dentry, mode_t mode, int devid);
    int     (*ifcntl)(inode_t *ip, int cmd, void *argp);
    int     (*iioctl)(inode_t *ip, int req, void *argp);
    int     (*imkdir)(inode_t *dir, struct dentry *dentry, mode_t mode);
    int     (*ilookup)(inode_t *dir, struct dentry *dentry);
    int     (*icreate)(inode_t *dir, struct dentry *dentry, mode_t mode);
    int     (*irename)(inode_t *dir, struct dentry *old, inode_t *newdir, struct dentry *new);
    ssize_t (*ireaddir)(inode_t *dir, off_t off, void *buf, size_t count);
    int     (*isymlink)(inode_t *ip, inode_t *atdir, const char *symname);
    int     (*igetattr)(inode_t *ip, void *attr);
    int     (*isetattr)(inode_t *ip, void *attr);
} iops_t;

#define iassert(ip)         ({ assert((ip), "No inode"); })
#define ilock(ip)           ({ iassert(ip); spin_lock(&(ip)->i_lock); })
#define iunlock(ip)         ({ iassert(ip); spin_unlock(&(ip)->i_lock); })
#define ilocked(ip)         ({ iassert(ip); spin_islocked(&(ip)->i_lock); })
#define iassert_locked(ip)  ({ iassert(ip); spin_assert_locked(&(ip)->i_lock); })

#define icheck_op(ip, func) ({                      \
    int err = 0;                              \
    iassert_locked(ip);                       \
    if ((ip) == NULL)                         \
        err = -EINVAL;                        \
    else if ((ip)->i_ops == NULL)             \
        err = -EINVAL;                        \
    else if ((ip)->i_ops->func == NULL) \
        err = -ENOSYS;                        \
    err;                                      \
})

#define IISTYPE(ip, type) ({\
    iassert_locked(ip);\
    (ip)->i_type == (type);\
})

#define IISINV(ip) ({ IISTYPE(ip, FS_INV); })
#define IISREG(ip) ({ IISTYPE(ip, FS_REG); })
#define IISDIR(ip) ({ IISTYPE(ip, FS_DIR); })
#define IISCHR(ip) ({ IISTYPE(ip, FS_CHR); })
#define IISSYM(ip) ({ IISTYPE(ip, FS_SYM); })
#define IISBLK(ip) ({ IISTYPE(ip, FS_BLK); })
#define IISFIFO(ip)({ IISTYPE(ip, FS_FIFO); })
#define IISDEV(ip) ({ IISCHR(ip) || IISBLK(ip); })

int ialloc(inode_t **pip);
void iputcnt(inode_t *ip);
void idupcnt(inode_t *ip);
void iputlink(inode_t *ip);
void iduplink(inode_t *ip);

int     isync(inode_t *ip);
int     iclose(inode_t *ip);
int     iunlink(inode_t *ip);
int     ibind(inode_t *dir, struct dentry *dentry, inode_t *ip);
int     ilink(struct dentry *oldname, inode_t *dir, struct dentry *newname);
ssize_t iread(inode_t *ip, __off_t off, void *buf, size_t nb);
ssize_t iwrite(inode_t *ip, __off_t off, void *buf, size_t nb);
int     imknod(inode_t *dir, struct dentry *dentry, mode_t mode, int devid);
int     ifcntl(inode_t *ip, int cmd, void *argp);
int     iioctl(inode_t *ip, int req, void *argp);
int     imkdir(inode_t *dir, struct dentry *dentry, mode_t mode);
int     ilookup(inode_t *dir, struct dentry *dentry);
int     icreate(inode_t *dir, struct dentry *dentry, mode_t mode);
int     irename(inode_t *dir, struct dentry *old, inode_t *newdir, struct dentry *new);
ssize_t ireaddir(inode_t *dir, off_t off, void *buf, size_t count);
int     isymlink(inode_t *ip, inode_t *atdir, const char *symname);
int     igetattr(inode_t *ip, void *attr);
int     isetattr(inode_t *ip, void *attr);
int     itruncate(inode_t *ip);