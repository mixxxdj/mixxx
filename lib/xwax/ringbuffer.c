#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"

/*
 * Allocates the ringbuffer
 */

struct ringbuffer *rb_alloc(size_t size, size_t elem_size)
{
    if (!size || !elem_size) {
        errno = EINVAL;
        perror(__func__);
        return NULL;
    }

    struct ringbuffer *rb = malloc(sizeof(struct ringbuffer));
    if (!rb)
        goto error;

    rb->size = size;
    rb->elem_size = elem_size;
    rb->current = size - 1;

    rb->data = calloc(size, elem_size);
    if (!rb->data)
        goto error_data;

    return rb;

error_data:
    free(rb);
error:
    perror(__func__);

    return NULL;
}

/*
 * Frees the ringbuffer
 */

void rb_free(struct ringbuffer *rb)
{
    if (!rb)
        return;

    if (rb->data) {
        free(rb->data);
        rb->data = NULL;
    }

    free(rb);
    rb = NULL;
}

/*
 * Gets the sample at index i in the ringbuffer
 */

const void *rb_at(const struct ringbuffer *rb, ptrdiff_t i)
{
    if (!rb || !rb->data || rb->size == 0) {
        errno = EINVAL;
        perror(__func__);
        return NULL;
    }

    ptrdiff_t index = (rb->current + i) % (ptrdiff_t)rb->size;
    while (index < 0)
        index += rb->size;

    /* Offset from the start in bytes: index * sizeof(element) */

    return (char *)rb->data + index * rb->elem_size;
}

/*
 * Decrements the ringbuffer pointer
 */

static inline void rb_dec(struct ringbuffer *rb)
{
    if (!rb || rb->size == 0) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    rb->current--;
    while (rb->current < 0)
        rb->current += rb->size;
}

/*
 * Pushes a new sample to the ringbuffer
 */

void rb_push(struct ringbuffer *rb, const void *elem)
{
    if (!rb || !rb->data) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    rb_dec(rb);

    /* Compute the destination pointer as an offset in bytes from the start of the buffer */

    memcpy((char *)rb->data + rb->current * rb->elem_size, elem, rb->elem_size);
}
