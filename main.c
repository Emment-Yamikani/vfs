#include <fs.h>
#include <generic.h>
#include <pthread.h>
#include <queue.h>


int main(void) {
    __unused dentry_t *dir = NULL, *d = NULL;
    vfs_init();

    vfs_mount(NULL, "/tmp", "tmpfs", 0, NULL);
    vfs_mount(NULL, "/mnt", "tmpfs", 0, NULL);


    return 0;
}

