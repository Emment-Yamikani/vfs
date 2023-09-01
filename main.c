#include <fs.h>

int main(void) {
    dentry_t *droot = NULL;
    dentry_t *d0 = NULL, *d1 = NULL, *dentry = NULL;

    dalloc("/", &droot);

    dalloc("tmp", &d0);
    dbind(droot, d0);
    dunlock(d0);

    dalloc("mnt", &d1);
    dbind(droot, d1);
    dunlock(d1);

    dlookup(droot, "tmp", &dentry);
    
    return 0;
}

