#include <fs.h>
#include <errno.h>
#include <generic.h>


static spinlock_t *fsIDlock = &SPINLOCK_INIT();
static atomic_t fsIDalloc(void) {
    static atomic_t fs_ID = 0;
    spin_lock(fsIDlock);
    atomic_t id = __atomic_add_fetch(&fs_ID, 1, __ATOMIC_SEQ_CST);
    spin_unlock(fsIDlock);
    return id;
}

int fsalloc(filesystem_t **pfs) {
    int err = 0;
    queue_t *queue = NULL;
    filesystem_t *fs = NULL;

    if (pfs == NULL)
        return -EINVAL;

    if ((err = qalloc(&queue)))
        return err;

    if ((fs = malloc(sizeof *fs)) == NULL) {
        err = -ENOMEM;
        goto error;
    }

    memset(fs, 0, sizeof *fs);

    fs->fs_count = 1;
    fs->fs_id = fsIDalloc();
    fs->fs_lock = SPINLOCK_INIT();
    fslock(fs);
    *pfs = fs;
    return 0;
error:
    if (queue)
        qfree(queue);
    return err;
}

void fs_free(filesystem_t *fs) {
    fsassert_locked(fs);

    if (fs->fs_count <= 0) {
        fs_unsetname(fs);
        qfree(fs->fs_superblocks);
        fsunlock(fs);
        free(fs);
    }

    fs_put(fs);
    fsunlock(fs);
}

void fs_dup(filesystem_t *fs) {
    fsassert_locked(fs);
    fs->fs_count++;
}

void fs_put(filesystem_t *fs) {
    fsassert_locked(fs);
    fs->fs_count--;
}

int fs_setname(filesystem_t *fs, const char *fsname) {
    char *name = NULL;

    fsassert_locked(fs);

    if (fs == NULL || fsname == NULL)
        return -EINVAL;

    if ((name = strdup(fsname)) == NULL)
        return -ENOMEM;
    
    fs->fs_name = name;

    return 0;
}

void fs_unsetname(filesystem_t *fs) {
    fsassert_locked(fs);
    if (fs == NULL)
        return;
    if (fs->fs_name)
        free(fs->fs_name);
}

int fs_set_iops(filesystem_t *fs, iops_t *iops) {
    fsassert_locked(fs);
    if (fs == NULL || iops == NULL)
        return -EINVAL;
    fs->fs_iops = iops;
    return 0;
}