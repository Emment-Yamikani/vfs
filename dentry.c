#include <dentry.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <generic.h>

void ddump(dentry_t *dentry, int flags) {
    dassert_locked(dentry);

    printf(
        "d_name: %s\nd_refs: %ld\nd_flags: %lX\n"
        "d_next: %p\nd_prev: %p\nd_parent: %p\nd_children: %p\n",
        dentry->d_name,
        dentry->d_refs,
        dentry->d_flags,
        dentry->d_next,
        dentry->d_prev,
        dentry->d_parent,
        dentry->d_child
    );

    if (flags & DDUMP_HANG)
        while(1);
    else if (flags & DDUMP_PANIC)
        panic("PANICKED\n");
}

int dalloc(const char *__name, dentry_t **pdentry) {
    int err = 0;
    char *name = NULL;
    dentry_t *dentry = NULL;

    if (__name == NULL || pdentry == NULL)
        return -EINVAL;
    
    if (NULL == (name = strdup(__name)))
        return -ENOMEM;
    
    err = -ENOMEM;
    if (NULL == (dentry = malloc(sizeof *dentry)))
        goto error;

    memset(dentry, 0, sizeof *dentry);

    dentry->d_refs = 0;
    dentry->d_name = name;
    dentry->d_lock = SPINLOCK_INIT();

    dlock(dentry);
    *pdentry = dentry;
    return 0;
error:
    if (name)
        free(name);
    if (dentry)
        free(dentry);
    return err;
}

static void dfree(dentry_t *dentry) {
    dassert_locked(dentry);
    dunbind(dentry);
    if (dentry->d_name)
        free(dentry->d_name);
    free(dentry);
}

void ddup(dentry_t *dentry) {
    dassert_locked(dentry);
    dentry->d_refs++;
}

void drelease(dentry_t *dentry) {
    dassert_locked(dentry);
    if (--dentry->d_refs <= 0)
        dfree(dentry);
}

void dunbind(dentry_t *dentry) {
    dentry_t *next = NULL;
    dentry_t *prev = NULL;
    dentry_t *d_parent = NULL;

    dassert_locked(dentry);

    next = dentry->d_next;
    prev = dentry->d_prev;
    d_parent = dentry->d_parent;

    if (prev) {
        dlock(prev);
        prev->d_next = next;
        if (next == NULL)
            drelease(next);
        else {
            dlock(next);
            next->d_prev = prev;
            dunlock(next);
        }
        dunlock(prev);
        drelease(dentry);
        dentry->d_prev = NULL;
    }

    if (next) {
        dlock(next);
        next->d_prev = prev;
        if (prev == NULL)
            drelease(next);
        else {
            dlock(prev);
            prev->d_next = next;
            dunlock(prev);
        }
        dunlock(next);
        drelease(dentry);
        dentry->d_next = NULL;
    }

    if (d_parent) {
        dlock(d_parent);
        if (d_parent->d_child == dentry){
            d_parent->d_child = next ? next : prev;
            drelease(dentry);
        }
        drelease(d_parent);
        dentry->d_parent = NULL;
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
        (d_parent->d_name == NULL) ||
        (d_child->d_name == NULL))
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
    ddup(d_last);
    dunlock(d_last);
done:
    ddup(d_child);
    d_child->d_next = NULL;
    d_child->d_parent = d_parent;
    ddup(d_parent);
    return 0;
}

void dclose(dentry_t *dentry) {
    dassert_locked(dentry);
    dunbind(dentry);
    drelease(dentry);
}

int dlookup(dentry_t *d_parent, const char *name, dentry_t **pchild) {
    dentry_t *dentry = NULL, *d_next = NULL;

    dassert_locked(d_parent);

    if (name == NULL)
        return -EINVAL;
    
    if (!compare_strings(".", name)) {
        dentry = d_parent;
        goto done;
    } else if (!compare_strings("..", name)) {
        dentry = d_parent->d_parent;
        if (dentry)
            dlock(dentry);
        else
            return 0;
        goto done;
    }

    forlinked(d_node, d_parent->d_child, d_next) {
        dlock(d_node);
        d_next = d_node->d_next;
        if (!compare_strings(d_node->d_name, name)) {
            dentry = d_node;
            goto done;
        }
        dunlock(d_node);
    }

    return -ENOENT;
done:
    if (pchild) {
        ddup(dentry);
        *pchild = dentry;
    } else
        dunlock(dentry);
    return 0;
}