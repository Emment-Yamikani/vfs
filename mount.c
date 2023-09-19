#include <fs.h>
#include <generic.h>

static queue_t *mountpoints = &QUEUE_INIT();

int vfs_mount(const char *src, const char *dest __unused,
              const char *type, unsigned long flags,
              const void *data __unused) {
    int err = 0;
    int no_dev = 0;
    filesystem_t *fs = NULL;
    fs_mount_t *mount = NULL;
    inode_t *ipsrc = NULL, *ipdst = NULL;
    dentry_t *dentry = NULL, *dsrc = NULL, *ddst = NULL;


    if (src == NULL)
        return -EINVAL;
    
    if (!compare_strings(src, "none"))
        no_dev = 1;

    if ((err = vfs_getfs(type, &fs)))
        return err;
    
    if (flags & MS_REMOUNT) {

    }

    if (flags & MS_BIND) {
        if (((err = vfs_lookup(src, NULL, 0, &dsrc))))
            return err;
        ipsrc = dsrc->d_inode;
        ilock(ipsrc);
        if (IISDIR(ipsrc) == 0) {
            iunlock(ipsrc);
            dclose(dsrc);
            fsunlock(fs);
            return -ENOTDIR;
        }
    }

    if (flags & MS_MOVE) {

    }

    return -ENOSYS;
}