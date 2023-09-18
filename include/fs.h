#include <dentry.h>
#include <inode.h>
#include <mount.h>
#include <generic.h>
#include <spinlock.h>
#include <queue.h>

#define MAXFNAME 255

typedef struct uio {
    uid_t   u_uid, u_euid, u_suid;
    gid_t   u_gid, u_egid, u_sgid;
    mode_t  u_umask;
} uio_t;

struct filesystem;
struct superblock;

typedef struct {
    inode_t *(*inode_alloc)(struct superblock *);
    dentry_t *(*mount)(struct superblock *);
    int (*unmount)(struct superblock *);
    int (*getattr)(struct superblock *);
    int (*setattr)(struct superblock *);
} sb_ops_t;

typedef struct superblock {
    long                sb_id;
    uio_t               sb_uio;
    sb_ops_t            sb_ops;
    long                sb_count;
    uintptr_t           sb_magic;
    uintptr_t           sb_flags;
    uintptr_t           sb_iflags;
    void                *sb_priv;
    dentry_t            *sb_root;
    size_t              sb_blocksize;
    fs_mount_t          *sb_mountpoint;
    struct filesystem   *sb_filesystem;
    spinlock_t          sb_lock;
} superblock_t;

#define sbassert(sb)        ({ assert((sb), "No superblock"); })
#define sblock(sb)          ({ sbassert(sb); spin_lock(&(sb)->sb_lock); });
#define sbunlock(sb)        ({ sbassert(sb); spin_unlock(&(sb)->sb_lock); })
#define sblocked(sb)        ({ sbassert(sb); spin_locked(&(sb)->sb_lock); })
#define sbassert_locked(sb) ({ sbassert(sb); spin_assert_locked(&(sb)->sb_lock); })

typedef struct {
    int     (*init)();
    int     (*fini)();
} fs_ops_t;

typedef struct filesysten {
    long        fs_id;
    fs_ops_t     fs_ops;
    long        fs_flags;
    long        fs_count;
    char        *fs_name;
    iops_t      *fs_iops;
    void        *fs_priv;
    queue_t     *fs_superblocks;
    spinlock_t  fs_lock;
} filesystem_t;

#define fsassert(fs)        ({ assert((fs), "No filesystem"); })
#define fslock(fs)          ({ fsassert(fs); spin_lock(&(fs)->fs_lock); });
#define fsunlock(fs)        ({ fsassert(fs); spin_unlock(&(fs)->fs_lock); })
#define fsislocked(fs)      ({ fsassert(fs); spin_islocked(&(fs)->fs_lock); })
#define fsassert_locked(fs) ({ fsassert(fs); spin_assert_locked(&(fs)->fs_lock); })

void fs_dup(filesystem_t *fs);
void fs_put(filesystem_t *fs);
void fs_free(filesystem_t *fs);
long fs_count(filesystem_t *fs);
void fs_unsetname(filesystem_t *fs);
int fs_set_iops(filesystem_t *fs, iops_t *iops);
int fs_setname(filesystem_t *fs, const char *fsname);
int fs_create(const char *name, iops_t *iops, filesystem_t **pfs);


int verify_path(const char *path);
int parse_path(const char *path, const char *__cwd, char **__abspath, char ***__abspath_tokens, char **__last_token);

int vfs_init(void);
dentry_t *vfs_getdroot(void);
int vfs_register_fs(filesystem_t *fs);
int vfs_unregister_fs(filesystem_t *fs);
int  vfs_getfs(const char *type, filesystem_t **pfs);
int vfs_lookup(const char *fn, const char *cwd, int oflags __unused, dentry_t **pdp);