#include <fs.h>
#include <errno.h>
#include <btree.h>

static inode_t *tmpfs_iroot = NULL;
static filesystem_t *tmpfs = NULL;
__unused static superblock_t *tmpfs_sb = NULL;

int tmpfs_init(void);
int tmpfs_fini(void) __unused;
static int tmpfs_ialloc(itype_t type, inode_t **pip);

__unused static fs_ops_t fsops = {
    .fini = tmpfs_fini,
    .init = NULL,
};

static iops_t tmpfs_iops = {
    NULL
};

int tmpfs_init(void) {
    int err = 0;

    if ((err = fs_create("tmpfs", &tmpfs_iops, &tmpfs)))
        return err;

    if ((err = tmpfs_ialloc(FS_DIR, &tmpfs_iroot)))
        goto error;

    if ((err = vfs_register_fs(tmpfs)))
        goto error;

    return 0;
error:
    if (tmpfs)
        fs_free(tmpfs);
    return err;
}

int tmpfs_fini(void) {
    return -EBUSY;
}

typedef struct {
    uio_t       uio;
    uintptr_t   ino;
    itype_t     type;
    size_t      size;
    void        *data;
    void        *priv;
} tmpfs_inode_t;

typedef struct {
    size_t         d_len;
    uintptr_t      d_ino;
    char           d_name[];
} tmpfs_dirent_t;

static int tmpfs_ialloc(itype_t type, inode_t **pip) {
    int err =  0;
    btree_t *bt = NULL;
    inode_t *inode = NULL;
    static long tmpfs_inos = 0;
    tmpfs_inode_t *tmpfs_ip = NULL;

    if (pip == NULL)
        return -EINVAL;

    if ((tmpfs_ip = malloc(sizeof *tmpfs_ip)) == NULL)
        return ENOMEM;

    if (type == FS_DIR) {
        if ((err = btree_alloc(&bt)))
            goto error;
    }

    memset(tmpfs_ip, 0, sizeof *tmpfs_ip);

    if ((err = ialloc(&inode)))
        goto error;

    inode->i_type = type;
    inode->i_priv = tmpfs_ip;

    tmpfs_ip->data = bt;
    tmpfs_ip->type= type;
    tmpfs_ip->priv = inode;
    tmpfs_ip->ino = __atomic_fetch_add(&tmpfs_inos, 1, __ATOMIC_SEQ_CST);

    *pip = inode;
    return 0;
error:
    if (tmpfs_ip)
        free(tmpfs_ip);
    
    if (bt)
        btree_free(bt);
    return err;
}