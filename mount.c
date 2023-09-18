#include <fs.h>
#include <generic.h>

static queue_t *mountpoints = &QUEUE_INIT();

int vfs_mount(const char *src __unused, const char *dest __unused,
              const char *type, unsigned long flags __unused,
              const void *data __unused) {
    int err = 0;
    dentry_t *dentry = NULL;
    filesystem_t *fs = NULL;
    fs_mount_t *mount = NULL;

    if ((err = vfs_getfs(type, &fs)))
        return err;

    
    return -ENOSYS;
}