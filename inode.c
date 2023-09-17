#include <inode.h>
#include <fs.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dentry.h>

int ialloc(inode_t **pip) {
    int err = -ENOMEM;
    inode_t *ip = NULL;

    if (pip == NULL)
        return -EINVAL;

    if ((ip = malloc(sizeof *ip)) == NULL)
        return err;

    memset(ip, 0, sizeof *ip);

    ip->i_lock = SPINLOCK_INIT();
    ip->i_count = 1;
    ilock(ip);
    *pip = ip;

    return 0;
}

void ifree(inode_t *ip) {
    iassert_locked(ip);

    if (ip->i_count <= 0) {
        iunlink(ip);
        free(ip);
    }
}

void idupcnt(inode_t *ip) {
    iassert_locked(ip);
    ip->i_count++;
}

void iputcnt(inode_t *ip) {
    iassert_locked(ip);
    ip->i_count--;
}

void iputlink(inode_t *ip) {
    iassert_locked(ip);
    ip->i_links--;
}

void iduplink(inode_t *ip) {
    iassert_locked(ip);
    ip->i_links++;
}

int     ibind(inode_t *dir, struct dentry *dentry, inode_t *ip) {
    int err = 0;
    iassert_locked(dir);
    iassert_locked(ip);
    dassert_locked(dentry);

    if (IISDIR(dir) == 0)
        return -ENOTDIR;
    
    if ((err = icheck_op(dir, ibind)))
        return err;
    
    return dir->i_ops->ibind(dir, dentry, ip);
}

int     isync(inode_t *ip) {
    int err = 0;
    iassert_locked(ip);

    if ((err = icheck_op(ip, isync)))
        return err;
    
    return ip->i_ops->isync(ip);
}

int     ilink(struct dentry *oldname, inode_t *dir, struct dentry *newname) {
    int err = 0;
    iassert_locked(dir);
    dassert_locked(oldname);
    dassert_locked(newname);

    if (IISDIR(dir) == 0)
        return -ENOTDIR;

    if ((err = icheck_op(dir, ilink)))
        return err;
    
    return dir->i_ops->ilink(oldname, dir, newname);
}

int     iclose(inode_t *ip) {
    int err = 0;
    iassert_locked(ip);

    if ((err = icheck_op(ip, iclose)))
        return err;
    
    return ip->i_ops->iclose(ip);
}

ssize_t iread(inode_t *ip, off_t off, void *buf, size_t nb) {
    ssize_t err = 0;
    iassert_locked(ip);

    if (IISDIR(ip))
        return -EISDIR;

    if ((err = icheck_op(ip, iread)))
        return err;
    
    return ip->i_ops->iread(ip, off, buf, nb);
}

ssize_t iwrite(inode_t *ip, off_t off, void *buf, size_t nb) {
    ssize_t err = 0;
    iassert_locked(ip);

    if (IISDIR(ip))
        return -EISDIR;

    if ((err = icheck_op(ip, iwrite)))
        return err;
    
    return ip->i_ops->iwrite(ip, off, buf, nb);
}

int     imknod(inode_t *dir, struct dentry *dentry, mode_t mode, int devid) {
    int err = 0;
    iassert_locked(dir);
    dassert_locked(dentry);

    if (IISDIR(dir) == 0)
        return -ENOTDIR;

    if ((err = icheck_op(dir, imknod)))
        return err;
    
    return dir->i_ops->imknod(dir, dentry, mode, devid);
}

int     ifcntl(inode_t *ip, int cmd, void *argp) {
    int err = 0;
    iassert_locked(ip);

    if (argp == NULL)
        return -EINVAL;

    if ((err = icheck_op(ip, ifcntl)))
        return err;
    
    return ip->i_ops->ifcntl(ip, cmd, argp);
}

int     iioctl(inode_t *ip, int req, void *argp) {
    int err = 0;
    iassert_locked(ip);

    if (argp == NULL)
        return -EINVAL;

    if (IISDIR(ip))
        return -EISDIR;

    if ((err = icheck_op(ip, iioctl)))
        return err;
    
    return ip->i_ops->iioctl(ip, req, argp);
}

int     imkdir(inode_t *dir, struct dentry *dentry, mode_t mode) {
    int err = 0;
    iassert_locked(dir);
    dassert_locked(dentry);
    
    if (IISDIR(dir) == 0)
        return -ENOTDIR;

    if ((err = icheck_op(dir, imkdir)))
        return err;
    
    return dir->i_ops->imkdir(dir, dentry, mode);
}

int     iunlink(inode_t *ip) {
    int err = 0;
    iassert_locked(ip);

    if ((err = icheck_op(ip, iunlink)))
        return err;
    
    return ip->i_ops->iunlink(ip);
}

int     ilookup(inode_t *dir, struct dentry *dentry) {
    int err = 0;
    
    iassert_locked(dir);
    dassert_locked(dentry);

    if (IISDIR(dir) == 0)
        return -ENOTDIR;

    if ((err = icheck_op(dir, ilookup)))
        return err;
    
    return dir->i_ops->ilookup(dir, dentry);
}

int     icreate(inode_t *dir, struct dentry *dentry, mode_t mode) {
    int err = 0;
    iassert_locked(dir);
    dassert_locked(dentry);

    if (IISDIR(dir) == 0)
        return -ENOTDIR;

    if ((err = icheck_op(dir, icreate)))
        return err;
    
    return dir->i_ops->icreate(dir, dentry, mode);
}

int     irename(inode_t *dir, struct dentry *old, inode_t *newdir, struct dentry *new) {
    int err = 0;
    iassert_locked(dir);
    iassert_locked(newdir);
    dassert_locked(old);
    dassert_locked(new);

    if ((err = icheck_op(dir, irename)))
        return err;
    
    return dir->i_ops->irename(dir, old, newdir, new);
}

ssize_t ireaddir(inode_t *dir, off_t off, void *buf, size_t count) {
    ssize_t err = 0;
    iassert_locked(dir);

    if (IISDIR(dir) == 0)
        return -ENOTDIR;

    if (buf == NULL)
        return -EINVAL;

    if (off < 0)
        return -ERANGE;

    if ((err = icheck_op(dir, ireaddir)))
        return err;
    
    return dir->i_ops->ireaddir(dir, off, buf, count);
}

int     isymlink(inode_t *ip, inode_t *atdir, const char *symname) {
    int err = 0;
    iassert_locked(ip);
    if ((err = icheck_op(ip, isymlink)))
        return err;
    
    return ip->i_ops->isymlink(ip, atdir, symname);
}

int     igetattr(inode_t *ip, void *attr) {
    int err = 0;
    iassert_locked(ip);
    if ((err = icheck_op(ip, igetattr)))
        return err;
    
    return ip->i_ops->igetattr(ip, attr);
}

int     isetattr(inode_t *ip, void *attr) {
    int err = 0;
    iassert_locked(ip);
    if ((err = icheck_op(ip, isetattr)))
        return err;
    
    return ip->i_ops->isetattr(ip, attr);
}

int     itruncate(inode_t *ip) {
    int err = 0;
    iassert_locked(ip);

    if (IISDIR(ip))
        return -EISDIR;

    if ((err = icheck_op(ip, itruncate)))
        return err;
    
    return ip->i_ops->itruncate(ip);
}
