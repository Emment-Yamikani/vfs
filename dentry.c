#include <dentry.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <generic.h>

void ddump(dentry_t *dp, int flags) {
    printf(
        "d_name: %p::%s\nd_count: %ld\nd_flags: %lX\n"
        "d_next: %p\nd_prev: %p\nd_parent: %p\nd_children: %p\n\n",
        dp,
        dp->d_name,
        dp->d_count,
        dp->d_flags,
        dp->d_next,
        dp->d_prev,
        dp->d_parent,
        dp->d_child
    );

    if (flags & DDUMP_HANG)
        while(1);
    else if (flags & DDUMP_PANIC)
        panic("PANICKED\n");
}

long dget_count(dentry_t *dp) {
    dassert_locked(dp);
    return dp->d_count;
}

void ddup(dentry_t *dp) {
    dassert_locked(dp);
    ++dp->d_count;
}

void dput(dentry_t *dp) {
    dassert_locked(dp);
    --dp->d_count;
}

void diput(dentry_t *dp) {
    dassert_locked(dp);
}

int ddelete(struct dentry *dp) {
    dassert_locked(dp);
    printf("delete dentry object\n");
    return -ENOSYS;
}

void drelease(struct dentry *dp) {
    dassert_locked(dp);

}

int drevalidate(struct dentry *dp) {
    dassert_locked(dp);
    return dp->d_inode ? 1 : 0;
}

int dalloc(const char *__name, dentry_t **pdentry) {
    int err = 0;
    char *name = NULL;
    dentry_t *dp = NULL;

    if (__name == NULL || pdentry == NULL)
        return -EINVAL;
    
    if (NULL == (name = strdup(__name)))
        return -ENOMEM;
    
    err = -ENOMEM;
    if (NULL == (dp = malloc(sizeof *dp)))
        goto error;

    memset(dp, 0, sizeof *dp);

    dp->d_count = 0;
    dp->d_name = name;
    dp->d_lock = SPINLOCK_INIT();
    dp->d_ops = (dops_t) {
        .diput = diput,
        .ddelete = ddelete,
        .drelease = drelease,
        .drevalidate = drevalidate,
    };

    dlock(dp);
    *pdentry = dp;
    return 0;
error:
    if (name)
        free(name);
    if (dp)
        free(dp);
    return err;
}

void dfree(dentry_t *dp) {
    dassert_locked(dp);
    dunbind(dp);
    dunlock(dp);
    if (dp->d_name)
        free(dp->d_name);
    free(dp);
}

void dunbind(dentry_t *dp) {
    dentry_t *next = NULL;
    dentry_t *prev = NULL;
    dentry_t *d_parent = NULL;

    dassert_locked(dp);
    next = dp->d_next;
    prev = dp->d_prev;
    d_parent = dp->d_parent;

    if (prev) {
        dlock(prev);
        prev->d_next = next;
        if (next) {
            dlock(next);
            next->d_prev = prev;
            dunlock(next);
        }
        dunlock(prev);
        dp->d_prev = NULL;
    }

    if (next) {
        dlock(next);
        next->d_prev = prev;
        if (prev) {
            dlock(prev);
            prev->d_next = next;
            dunlock(prev);
        }
        dunlock(next);
        dp->d_next = NULL;
    }

    if (d_parent) {
        dlock(d_parent);
        if (d_parent->d_child == dp)
            d_parent->d_child = next ? next : prev;
        dp->d_parent = NULL;
        dunlock(d_parent);
    }
}

int dbind(dentry_t *d_parent, dentry_t *d_child) {
    dentry_t *next = NULL, *d_last = NULL;
    dassert_locked(d_child);
    dassert_locked(d_parent);

    if (d_child->d_parent)
        return -EALREADY;

    if ((d_child == d_parent) ||
        (d_child->d_name == NULL) ||
        (d_parent->d_name == NULL))
        return -EINVAL;

    /**
     * Check for d_child dentry in d_parent dentry.
     * If d_parent dentry already contains d_child dentry
     * fail and return -EEXIST.
    */
    forlinked(node, d_parent->d_child, next) {
        if (node == d_child)
            return -EEXIST;
        dlock(node);
        next = node->d_next;
        if (!compare_strings(node->d_name, d_child->d_name)) {
            dunlock(node);
            return -EEXIST;
        }
        dunlock(node);
    }

    if (d_parent->d_child == NULL) {
        d_parent->d_child = d_child;
        goto done;
    }

    forlinked(node, d_parent->d_child, next) {
        d_last = node;
        dlock(node);
        next = node->d_next;
        if (next)
            dunlock(node);
    }

    d_child->d_prev = d_last;
    d_last->d_next = d_child;
    dunlock(d_last);
done:
    d_child->d_next = NULL;
    d_child->d_parent = d_parent;
    // dsetflags(d_parent, DCACHE_REFERENCED);
    return 0;
}

void dclose(dentry_t *dp) {
    dassert_locked(dp);
    dput(dp);
    if (dget_count(dp) == 0) {
        dp->d_ops.ddelete(dp);
        dunlock(dp);
    } else if (dget_count(dp) < 0) {
        dfree(dp);
    }

    printf("[WARNING]: call d_inode->iput() to release the reference of d_inode.\n");
}

int dlookup(dentry_t *d_parent, const char *name, dentry_t **pchild) {
    dentry_t *dp = NULL, *d_next = NULL;

    dassert_locked(d_parent);

    if (name == NULL)
        return -EINVAL;
    
    if (!compare_strings(".", name)) {
        dp = d_parent;
        goto done;
    } else if (!compare_strings("..", name)) {
        dp = d_parent->d_parent;
        if (dp)
            dlock(dp);
        else
            return 0;
        goto done;
    }

    forlinked(d_node, d_parent->d_child, d_next) {
        dlock(d_node);
        d_next = d_node->d_next;
        if (!compare_strings(d_node->d_name, name)) {
            dp = d_node;
            goto done;
        }
        dunlock(d_node);
    }

    return -ENOENT;
done:
    if (pchild)
        *pchild = dp;
    else
        dunlock(dp);
    return 0;
}

int dmkdentry(dentry_t *dir, const char *name, dentry_t **pdp) {
    int err = 0;
    dentry_t *dp = NULL;
    dassert_locked(dir);

    if (pdp == NULL)
        return -EINVAL;

    if ((err = dalloc(name, &dp)))
        return err;
    
    if ((err = dbind(dir, dp)))
        goto error;

    *pdp = dp;
    return 0;
error:
    if (dp)
        dfree(dp);
    return err;
}