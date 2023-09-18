#include <fs.h>
#include <errno.h>

static inode_t *iroot = NULL;
static filesystem_t *tmpfs = NULL;
static superblock_t *tmpfs_sb = NULL;

int tmpfs_init(void);
int tmpfs_fini(void);

static fs_ops_t fsops = {
    .fini = tmpfs_fini,
    .init = NULL,
};

int     tmpfs_isync(inode_t *tmpfs_ip);
int     tmpfs_iclose(inode_t *tmpfs_ip);
int     tmpfs_iunlink(inode_t *tmpfs_ip);
int     tmpfs_ibind(inode_t *dir, struct dentry *dentry, inode_t *tmpfs_ip);
int     tmpfs_ilink(struct dentry *oldname, inode_t *dir, struct dentry *newname);
ssize_t tmpfs_iread(inode_t *tmpfs_ip, __off_t off, void *buf, size_t nb);
ssize_t tmpfs_iwrite(inode_t *tmpfs_ip, __off_t off, void *buf, size_t nb);
int     tmpfs_imknod(inode_t *dir, struct dentry *dentry, mode_t mode, int devid);
int     tmpfs_ifcntl(inode_t *tmpfs_ip, int cmd, void *argp);
int     tmpfs_iioctl(inode_t *tmpfs_ip, int req, void *argp);
int     tmpfs_imkdir(inode_t *dir, struct dentry *dentry, mode_t mode);
int     tmpfs_ilookup(inode_t *dir, struct dentry *dentry);
int     tmpfs_icreate(inode_t *dir, struct dentry *dentry, mode_t mode);
int     tmpfs_irename(inode_t *dir, struct dentry *old, inode_t *newdir, struct dentry *new);
ssize_t tmpfs_ireaddir(inode_t *dir, off_t off, void *buf, size_t count);
int     tmpfs_isymlink(inode_t *tmpfs_ip, inode_t *atdir, const char *symname);
int     tmpfs_igetattr(inode_t *tmpfs_ip, void *attr);
int     tmpfs_isetattr(inode_t *tmpfs_ip, void *attr);
int     tmpfs_itruncate(inode_t *tmpfs_ip);

static iops_t tmpfs_iops = {
    NULL
};

int tmpfs_init(void) {
    int err = 0;

    if ((err = fs_create("tmpfs", &tmpfs_iops, &tmpfs)))
        return err;
    
    if ((err = vfs_register_fs(tmpfs)))
        return err;

    return 0;
}

int tmpfs_fini(void) {
    return -EBUSY;
}

typedef struct {
    uintptr_t   ino;
    uio_t       uio;
    size_t      size;
    void        *data;
    void        *priv;
} tmpfs_inode_t;

typedef struct {
    size_t         d_len;
    uintptr_t      d_ino;
    char           d_name[];
} tmpfs_dirent_t;

static int tmpfs_ialloc(tmpfs_inode_t **pip) {
    int err =  0;
    inode_t *inode = NULL;
    tmpfs_inode_t *tmpfs_ip = NULL;

    if (pip == NULL)
        return -EINVAL;

    if ((tmpfs_ip = malloc(sizeof *tmpfs_ip)) == NULL)
        return ENOMEM;

    memset(tmpfs_ip, 0, sizeof *tmpfs_ip);

    if ((err = ialloc(&inode)))
        goto error;

    inode->i_priv = tmpfs_ip;
    tmpfs_ip->priv = inode;

    *pip = tmpfs_ip;
    return 0;
error:
    if (tmpfs_ip)
        free(tmpfs_ip);
    return err;
}
