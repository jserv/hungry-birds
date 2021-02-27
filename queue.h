#ifndef MY_QUEUE_H
#define MY_QUEUE_H

#include <stdint.h>

typedef struct __QueueInternal *queue_p;

typedef enum __QueueResult {
    QUEUE_FALSE,
    QUEUE_SUCCESS,
    QUEUE_TRUE,
    QUEUE_OUT_OF_MEMORY = -1,
} QueueResult;

/**
 * \brief An unbounded lockless single consumer multiple producer FIFO Queue.
 */
extern struct __QUEUE_API__ {
    /** Create a new Queue object.
     * @param size the storage size in bytes.
     */
    queue_p (*create)(size_t size);

    /** Push an element to the back of the queue.
     * Pushing supports copying and moving. Pushing is considered a producer
     * operation. Any thread can safely execute this operation at any time.
     * @param data the region where the value stored will be copied from.
     * @return QUEUE_OUT_OF_MEMORY if the heap is exhausted.
     */
    QueueResult (*push)(queue_p, void *data);

    /** Check if the queue has any data.
     * The method is considered a consumer operation, and only one thread may
     * safely execute this at one time.
     * @return QUEUE_TRUE if there is a front.
     * @return QUEUE_FALSE if there is not.
     */
    QueueResult (*hasFront)(queue_p);

    /** Get the value at the front of the queue.
     * You should always check that there is data in queue before calling
     * front as there is no built in check. If no data is in the queue when
     * front is called, memory violation likely happens.
     * Getting data is considered a consumer operation, only one thread may
     * safely execute this at one time.
     * @param data the destination where value stored will be copied to
     */
    QueueResult (*front)(queue_p, void *data);

    /** Remove the item at the front of the queue.
     * You should always check that there is data in queue before popping as
     * there is no built in check. If no data is in the queue when pop is
     * called, memory violation likely happens.
     * Popping is considered a consumer operation, and only one thread may
     * safely execute this at one time.
     */
    QueueResult (*pop)(queue_p);

    /** Clear the entire queue.
     * You should always clear it before deleting the Queue itself, otherwise
     * you will leak memory.
     */
    QueueResult (*clear)(queue_p);

    /** Destroy the queue object */
    QueueResult (*destroy)(queue_p);
} Queue;

#endif
