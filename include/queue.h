#pragma once

#include <errno.h>
#include <string.h>
#include <generic.h>
#include <spinlock.h>

struct queue;

typedef struct queue_node {
    struct queue_node *prev;
    void              *data;
    struct queue_node *next;
    struct queue      *queue;
} queue_node_t;

typedef struct queue {
    queue_node_t    *head;
    queue_node_t    *tail;
    size_t          q_count;
    spinlock_t      q_lock;
} queue_t;

#define qassert(q)          ({ assert((q), "No queue"); })
#define qlock(q)            ({ qassert(q); spin_lock(&(q)->q_lock); })
#define qunlock(q)          ({ qassert(q); spin_unlock(&(q)->q_lock); })
#define qislocked(q)        ({ qassert(q); spin_islocked(&(q)->q_lock); })
#define qassert_locked(q)   ({ qassert(q);spin_assert_locked(&(q)->q_lock); })

static inline int qalloc(queue_t **pq) {
    queue_t *q = NULL;

    if (pq == NULL)
        return -EINVAL;

    if ((q = malloc(sizeof *q)) == NULL)
        return -ENOMEM;

    memset(q, 0, sizeof *q);
    q->q_lock = SPINLOCK_INIT();
    *pq = q;

    return 0;
}

static inline size_t qcount(queue_t *q) {
    qassert_locked(q);
    return q->q_count;
}

static inline queue_node_t *enqueue(queue_t *q, void *data) {
    queue_node_t *node = NULL;
    qassert_locked(q);
    if (q == NULL)
        return NULL;
    
    if ((node = malloc(sizeof (*node))) == NULL)
        return NULL;

    memset(node, 0, sizeof *node);

    node->data = data;

    if (q->head == NULL)
        q->head = node;
    else {
        q->tail->next = node;
        node->prev = q->tail;
    }

    q->tail = node;
    node->queue = q;
    q->q_count++;
    return node;
}

static inline void *dequeue(queue_t *q) {
    void *data = NULL;
    queue_node_t *node = NULL, *prev = NULL, *next = NULL;
    qassert_locked(q);

    if (q == NULL)
        return NULL;
    
    node = q->head;
    if (node) {
        data = node->data;
        prev = node->prev;
        next = node->next;

        if (prev)
            prev->next = next;
        if (next)
            next->prev = prev;
        if (node == q->head)
            q->head = next;
        if (node == q->tail)
            q->tail = prev;
        
        q->q_count--;
        node->queue = NULL;
        free(node);
    }

    return data;
}

static inline int qcontains(queue_t *q, void *data, queue_node_t **pnode) {
    queue_node_t *next = NULL;
    qassert_locked(q);
    if (q == NULL)
        return -EINVAL;
    
    forlinked (node, q->head, next) {
        next = node->next;
        if (node->data == data) {
            if (pnode)
                *pnode = node;
            return 0;
        }
    }

    return -ENOENT;
}

static inline int qremove_node(queue_t *q, queue_node_t *__node) {
    queue_node_t *next = NULL, *prev = NULL;
    qassert_locked(q);
    if (q == NULL || __node == NULL)
        return -EINVAL;
    
    forlinked (node, q->head, next) {
        next = node->next;
        prev = node->prev;
        if (__node == node) {
            if (prev)
                prev->next = next;
            if (next)
                next->prev = prev;
            if (node == q->head)
                q->head = next;
            if (node == q->tail)
                q->tail = prev;
            
            q->q_count--;
            node->queue = NULL;
            free(node);
            return 0;
        }
    }

    return -ENOENT;
}

static inline int qremove(queue_t *q, void *data) {
    queue_node_t *next = NULL, *prev = NULL;
    qassert_locked(q);

    if (q == NULL)
        return -EINVAL;
    
    forlinked (node, q->head, next) {
        next = node->next;
        prev = node->prev;
        if (node->data == data) {
            if (prev)
                prev->next = next;
            if (next)
                next->prev = prev;
            if (node == q->head)
                q->head = next;
            if (node == q->tail)
                q->tail = prev;
            
            q->q_count--;
            node->queue = NULL;
            free(node);
            return 0;
        }
    }

    return -ENOENT;
}

static inline void qflush(queue_t *q) {
    queue_node_t *next = NULL, *prev = NULL;
    qassert_locked(q);

    forlinked (node, q->head, next) {
        next = node->next;
        prev = node->prev;
        if (prev)
            prev->next = next;
        if (next)
            next->prev = prev;
        if (node == q->head)
            q->head = next;
        if (node == q->tail)
            q->tail = prev;

        q->q_count--;
        node->queue = NULL;
        free(node);
    }
}