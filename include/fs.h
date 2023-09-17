#include <dentry.h>
#include <inode.h>
#include <mount.h>
#include <generic.h>
#include <spinlock.h>

#define MAXFNAME 255

struct filesystem;

typedef struct superblock {
    long                sb_id;
    struct filesystem   *sb_fs;
    void                *sb_priv;

    spinlock_t          sb_lock;
} superblock_t;

typedef struct filesysten {
    long        fs_id;
    char        *fs_name;
    iops_t      *fs_iops;
    void        *fs_priv;

    spinlock_t  fs_lock;
} filesystem_t;

int verify_path(const char *path);
int parse_path(const char *path, const char *__cwd, char **__abspath, char ***__abspath_tokens, char **__last_token);

int vfs_init(void);
dentry_t *vfs_getdroot(void);
int vfs_lookup(const char *fn, const char *cwd, int oflags __unused, dentry_t **pdp);