#include <fs.h>
#include <generic.h>

int vfs_mount(const char *src __unused, const char *dest __unused,
              const char *type __unused, unsigned long flags __unused,
              const void *data __unused) {
    

    return -ENOSYS;
}