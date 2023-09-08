#include <fs.h>
#include <generic.h>
#include <pthread.h>


int main(void) {
    dentry_t *dir = NULL, *d = NULL;
    vfs_init();
    dir = vfs_getdroot();
    dmkdentry(dir, "bin", &d);
    dunlock(d);
    dmkdentry(dir, "tmp", &d);
    dunlock(d);
    dmkdentry(dir, "lib", &d);
    dunlock(d);
    dmkdentry(dir, "usr", &d);
    dunlock(d);
    dmkdentry(dir, "home", &d);
    dunlock(d);
    dmkdentry(dir, "mnt", &d);
    dunlock(dir);
    dir = d;
    dmkdentry(dir, "ramfs", &d);
    dunlock(dir);
    dir = d;
    dmkdentry(dir, "bin", &d);
    dunlock(dir);
    dir = d;
    dmkdentry(dir, "bash", &d);
    dunlock(d);
    dunlock(dir);

    vfs_lookup("/mnt/ramfs/bin/bash", NULL, 0, &d);
    ddump(d, 0);
    dunlock(d);

    vfs_lookup("/bin", NULL, 0, &d);
    ddump(d, 0);
    dunlock(d);

    vfs_lookup("/mnt/ramfs/bin", NULL, 0, &d);
    ddump(d, 0);
    dunlock(d);

    return 0;
}

