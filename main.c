#include <fs.h>
#include <generic.h>
#include <pthread.h>
#include <queue.h>
#include <tmpfs.h>


int main(void) {
    __unused int err = 0;
    __unused dentry_t *dir = NULL, *d = NULL;
    if ((err = vfs_init())){
        errno = -err;
        perror("");
        panic("error[%d]: Failed to initialize VFS\n", err);
    }

    if ((err = tmpfs_init())){
        errno = -err;
        perror("");
        panic("error[%d]: Failed to initialize tmpfs\n", err);
    }

    if ((err = vfs_mount("none", "/tmp", "tmpfs", MS_BIND, NULL))) {
        errno = -err;
        perror("");
        panic("error[%d]: Failed to mount tmpfs\n", err);
    }

    return 0;
}

