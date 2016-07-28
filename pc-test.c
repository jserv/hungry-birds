/*
 * Classical Producer-Consumer Problem, utiling unbounded lockless single consumer
 * multiple producer FIFO queue.
 */

#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "queue.h"

static void test_basic_operations()
{
    queue_p q = Queue.create(sizeof(int));
    assert(q);

    /* initial queue is empty */
    assert(Queue.hasFront(q) == QUEUE_FALSE);

    QueueResult result;
    /* push one item to the empty queue */
    {
        int in = 1, out = 0;
        {
            result = Queue.push(q, &in);
            assert(result == QUEUE_SUCCESS);
        }
        assert(Queue.hasFront(q) == QUEUE_TRUE);
        {
            result = Queue.front(q, &out);
            assert(result == QUEUE_SUCCESS);
        }
        assert(out == in);
    }

    /* pop one item out of a single item queue */
    {
        result = Queue.pop(q);
        assert(result == QUEUE_SUCCESS);
    }
    assert(Queue.hasFront(q) == QUEUE_FALSE);

    /* push many items on the queue */
    for (size_t i = 0; i < 64; ++i) {
        int in = i;
        int out = -1;
        result = Queue.push(q, &in);
        assert(result == QUEUE_SUCCESS);

        assert(Queue.hasFront(q) == QUEUE_TRUE);
        result = Queue.front(q, &out);
        assert(result == QUEUE_SUCCESS);

        assert(out == 0);
    }

    /* pop many items from the queue */
    for (size_t i = 0; i < 32; ++i) {
        int out = -1;
        result = Queue.front(q, &out);
        assert(result == QUEUE_SUCCESS);
        assert(out == i);

        result = Queue.pop(q);
        assert(result == QUEUE_SUCCESS);
    }

    /* clear the queue */
    assert(Queue.hasFront(q) == QUEUE_TRUE);
    result = Queue.clear(q);
    assert(result == QUEUE_SUCCESS);

    assert(Queue.hasFront(q) == QUEUE_FALSE);

    Queue.destroy(q);
}

#define THREAD_COUNT (1.5 * 1000 * 1000)
#define PRODUCER_COUNT 7

typedef struct __QueueTest {
    atomic_int consume_count;
    atomic_int producer_count;
    queue_p q;
} QueueTest;

static void *test_consumer(void *arg)
{
    QueueTest *test = (QueueTest *) arg;
    while (atomic_load(&test->consume_count) < THREAD_COUNT) {
        if (Queue.hasFront(test->q)) {
            atomic_fetch_add(&test->consume_count, 1);
            QueueResult result = Queue.pop(test->q);
            assert(result == QUEUE_SUCCESS);
        }
    }
    return NULL;
}

static void *test_producer(void *arg)
{
    QueueTest *test = (QueueTest *) arg;
    assert(test->q);
    while (1) {
        int in = atomic_fetch_add(&test->producer_count, 1);
        if (in >= THREAD_COUNT) break;
        QueueResult result = Queue.push(test->q, &in);
        assert(result == QUEUE_SUCCESS);
    }
    return NULL;
}

static void stress_test()
{
    QueueTest test;
    atomic_init(&test.consume_count, 0);
    atomic_init(&test.producer_count, 0);

    test.q = Queue.create(sizeof(int));
    assert(test.q);

    /* thread creation */
    pthread_t consumer;
    pthread_t producers[PRODUCER_COUNT];
    {
        int p_result =
            pthread_create(&consumer, NULL, test_consumer, &test);
        assert(p_result == 0);
    }
    for (size_t i = 0; i < PRODUCER_COUNT; ++i) {
        int p_result = pthread_create(&producers[i], NULL,
                                      test_producer, &test);
        assert(p_result == 0);
    }

    /* wait for completion */
    for (size_t i = 0; i < PRODUCER_COUNT; ++i) {
        int p_result = pthread_join(producers[i], NULL);
        assert(p_result == 0);
    }
    {
        int p_result = pthread_join(consumer, NULL);
        assert(p_result == 0);
    }

    assert(Queue.hasFront(test.q) == QUEUE_FALSE);

    Queue.destroy(test.q);
}

int main(int argc, char *argv[])
{
    printf("** Basic operations **\n");
    test_basic_operations();
    printf("Verified OK!\n\n");

    printf("** Stress test **\n");
    stress_test();
    printf("Verified OK!\n\n");

    return 0;
}
