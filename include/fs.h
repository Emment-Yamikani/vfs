#include <dentry.h>
#include <inode.h>
#include <mount.h>
#include <generic.h>
#include <spinlock.h>
#include <queue.h>

#define MAXFNAME 255

struct filesystem;

typedef struct superblock {
    long                sb_id;
    void                *sb_priv;
    long                sb_count;
    struct filesystem   *sb_filesystem;

    spinlock_t          sb_lock;
} superblock_t;

#define sbassert(sb)        ({ assert((sb), "No superblock"); })
#define sblock(sb)          ({ sbassert(sb); spin_lock(&(sb)->sb_lock); });
#define sbunlock(sb)        ({ sbassert(sb); spin_unlock(&(sb)->sb_lock); })
#define sblocked(sb)        ({ sbassert(sb); spin_locked(&(sb)->sb_lock); })
#define sbassert_locked(sb) ({ sbassert(sb); spin_assert_locked(&(sb)->sb_lock); })

typedef struct filesysten {
    long        fs_id;
    char        *fs_name;
    iops_t      *fs_iops;
    void        *fs_priv;
    long        fs_count;
    fs_mount_t  *fs_mountpoint;
    queue_t     *fs_superblocks;
    spinlock_t  fs_lock;
} filesystem_t;

#define fsassert(fs)        ({ assert((fs), "No filesystem"); })
#define fslock(fs)          ({ fsassert(fs); spin_lock(&(fs)->fs_lock); });
#define fsunlock(fs)        ({ fsassert(fs); spin_unlock(&(fs)->fs_lock); })
#define fslocked(fs)        ({ fsassert(fs); spin_locked(&(fs)->fs_lock); })
#define fsassert_locked(fs) ({ fsassert(fs); spin_assert_locked(&(fs)->fs_lock); })

void fs_dup(filesystem_t *fs);
void fs_put(filesystem_t *fs);
void fs_unsetname(filesystem_t *fs);
int fs_setname(filesystem_t *fs, const char *fsname);
int fs_set_iops(filesystem_t *fs, iops_t *iops);


int verify_path(const char *path);
int parse_path(const char *path, const char *__cwd, char **__abspath, char ***__abspath_tokens, char **__last_token);

int vfs_init(void);
dentry_t *vfs_getdroot(void);
int vfs_lookup(const char *fn, const char *cwd, int oflags __unused, dentry_t **pdp);