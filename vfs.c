
#include <fs.h>
#include <generic.h>
#include <tmpfs.h>

static dentry_t *droot = NULL;
static queue_t *fs_queue = &QUEUE_INIT();

dentry_t *vfs_getdroot(void) {
    if (droot)
        dlock(droot);
    return droot;
}

int vfs_init(void) {
    int err = 0;
    if ((err = dalloc("/", &droot)))
        return err;
    
    dunlock(droot);

    if ((err = tmpfs_init()))
        return err;
    
    
    return 0;
}

int vfs_lookup(const char *fn, const char *cwd, int oflags __unused, dentry_t **pdp) {
    int err = 0;
    size_t tok_i = 0;
    dentry_t *d_dir = NULL, *dp = NULL;
    char *path = NULL, *last_tok = NULL, **toks = NULL;

    if ((d_dir = vfs_getdroot()) == NULL)
        return -ENOENT;

    if ((err = verify_path(fn)))
        return err;
    
    if ((err = parse_path(fn, cwd, &path, &toks, &last_tok)))
        return err;

    if (!compare_strings(path, "/")) {
        dp = d_dir;
        goto found;
    }

    foreach(tok, toks) {
        dp = NULL;
        if ((err = dlookup(d_dir, tok, &dp)) == 0)
            goto next;
        else if (err == -ENOENT)
            goto delegate;
        else {
            dunlock(d_dir);
            goto error;
        }
    next:
        dunlock(d_dir);
        if (!compare_strings(tok, last_tok))
            goto found;
        d_dir = dp;
        tok_i++;
    }

delegate:
    dp = NULL;
    foreach(tok, &toks[tok_i]) {

    }

found:
    if (pdp) {
        ddup(dp);
        *pdp = dp;
    }
    else
        dclose(dp);
    return 0;
error:
    return err;
}

int vfs_register_fs(filesystem_t *fs) {
    int err = 0;
    fsassert_locked(fs);
    if (fs == NULL)
        return -EINVAL;

    if (fs->fs_ops.init) {
        if ((err = fs->fs_ops.init()))
            return err;
    }

    qlock(fs_queue);

    enqueue(fs_queue, fs);

    qunlock(fs_queue);
    
    return 0;
}

int vfs_unregister_fs(filesystem_t *fs) {
    fsassert_locked(fs);
    if (fs == NULL)
        return -EINVAL;

    if (fs_count(fs) > 0)
        return -EBUSY;
    
    return -EBUSY;
}
