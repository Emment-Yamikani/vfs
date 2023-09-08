#include <dentry.h>
#include <inode.h>
#include <mount.h>
#include <generic.h>

#define MAXFNAME 255

int verify_path(const char *path);
int parse_path(const char *path, const char *__cwd, char **__abspath, char ***__abspath_tokens, char **__last_token);

dentry_t *vfs_getdroot(void);
int vfs_init(void);
int vfs_lookup(const char *fn, const char *cwd, int oflags __unused, dentry_t **pdp);