#include <assert.h>
#include <malloc.h>
#include <stdatomic.h>
#include <string.h>
#include "queue.h"

static const size_t sentinel = 0xdeadc0de;
static const size_t alignment = sizeof(size_t);

typedef struct node {
    atomic_uintptr_t next;
} node;

struct __QueueInternal {
    atomic_uintptr_t head;
    atomic_uintptr_t tail;
    size_t item_size;
};

queue_p queue_create(size_t item_size)
{
    size_t *ptr = calloc(sizeof(struct __QueueInternal) + alignment, 1);
    ptr[0] = sentinel;
    queue_p q = (queue_p)(ptr + 1);
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    q->item_size = item_size;
    return q;
}

QueueResult queue_has_front(queue_p q)
{
    assert(q);
    if (atomic_load(&q->head) == 0)
        return QUEUE_FALSE;
    return QUEUE_TRUE;
}

QueueResult queue_front(queue_p q, void *data)
{
    assert(q);
    assert(data);
    node *head = (node *) atomic_load(&q->head);
    assert(head);
    memcpy(data, (void *)(head + 1), q->item_size);
    return QUEUE_SUCCESS;
}

QueueResult queue_pop(queue_p q)
{
    assert(q);
    assert(queue_has_front(q) == QUEUE_TRUE);

    /* get the head */
    node *popped = (node *) atomic_load(&q->head);
    node *compare = popped;

    /* set the tail and head to nothing if they are the same */
    if (atomic_compare_exchange_strong(&q->tail, &compare, 0)) {
        compare = popped;
        /* it is possible for another thread to have pushed after
         * we swap out the tail. In this case, the head will be different
         * then what was popped, so we just do a blind exchange regardless
         * of the result.
         */
        atomic_compare_exchange_strong(&q->head, &compare, 0);
    } else {
        /* tail is different from head, set the head to the next value */
        node *new_head = 0;
        while (!new_head) {
            /* it is possible that the next node has not been assigned yet,
             * so just spin until the pushing thread stores the value.
	     */
            new_head = (node *) atomic_load(&popped->next);
        }
        atomic_store(&q->head, (atomic_uintptr_t) new_head);
    }

    free(popped);
    return QUEUE_SUCCESS;
}

QueueResult queue_push(queue_p q, void *data)
{
    assert(q);
    /* create the new tail */
    node *new_tail = malloc(sizeof(node) + q->item_size);
    if (!new_tail) {
        return QUEUE_OUT_OF_MEMORY;
    }

    atomic_init(&new_tail->next, 0);
    memcpy(new_tail + 1, data, q->item_size);

    /* swap the new tail with the old */
    node *old_tail = (node *) atomic_exchange(&q->tail,
                                              (atomic_uintptr_t) new_tail);

    /* link the old tail to the new */
    if (old_tail) {
        atomic_store(&old_tail->next, (atomic_uintptr_t) new_tail);
    } else {
        atomic_store(&q->head, (atomic_uintptr_t) new_tail);
    }
    return QUEUE_SUCCESS;
}

QueueResult queue_clear(queue_p q)
{
    assert(q);
    while (queue_has_front(q) == QUEUE_TRUE) {
        QueueResult result = queue_pop(q);
        assert(result == QUEUE_SUCCESS);
    }
    return QUEUE_SUCCESS;
}

QueueResult queue_destroy(queue_p q)
{
    size_t *ptr = (size_t*)q - 1;
    assert(ptr[0] == sentinel);
    free(ptr);
    return QUEUE_SUCCESS;
}

/* API gateway */
struct __QUEUE_API__ Queue = {
    .create = queue_create,
    .hasFront = queue_has_front,
    .front = queue_front,
    .pop = queue_pop,
    .push = queue_push,
    .clear = queue_clear,
    .destroy = queue_destroy,
};
