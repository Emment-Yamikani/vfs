#include <fs.h>
#include <generic.h>


int vfs_mount(const char *src, const char *target,
              const char *type, unsigned long flags,
              const void *data __unused) {
    int err = 0;
    int has_dev = 0;
    filesystem_t *fs = NULL;
    inode_t *isrc = NULL, *itarget = NULL;
    __unused dentry_t *dentry = NULL, *dsrc = NULL, *dtarget = NULL;

    if (src == NULL)
        return -EINVAL;
    
    if (!compare_strings(src, "none"))
        has_dev = 1;

    if ((err = vfs_getfs(type, &fs)))
        return err;
    
    if (flags & MS_REMOUNT) {

    }

    if (flags & MS_BIND) {
        if (!has_dev){
            if (((err = vfs_lookup(src, NULL, 0, &dsrc))))
                return err;
            isrc = dsrc->d_inode;
            ilock(isrc);
            if (IISDIR(isrc) == 0) {
                iunlock(isrc);
                dclose(dsrc);
                fsunlock(fs);
                return -ENOTDIR;
            }
        }

        printf("Looking up target[%s]\n", target);
        if ((err = vfs_lookup(target, NULL, 0, &dtarget)))
            return err; // TODO: release dsrc

        itarget = dsrc->d_inode;
        ilock(itarget);
        if (IISDIR(itarget) == 0) {
            iunlock(itarget);
            dclose(dtarget);
            fsunlock(fs);
            return -ENOTDIR;
        }

        if ((err = fs->fs_ops.mount(fs, dsrc, dtarget, flags, (void *)data)))
            return err; // TODO: release resource
        
    }

    if (flags & MS_MOVE) {

    }

    return -ENOSYS;
}