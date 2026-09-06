#ifndef RINGBUFFER_H

#define RINGBUFFER_H

#include <stddef.h>

struct ringbuffer {
    size_t size;
    size_t elem_size;
    void *data;
    ptrdiff_t current;
};

struct ringbuffer *rb_alloc(size_t size, size_t elem_size);
void rb_free(struct ringbuffer *rb);

const void *rb_at(const struct ringbuffer *rb, ptrdiff_t i);
void rb_push(struct ringbuffer *rb, const void *elem);

#endif /* end of include guard RINGBUFFER_H */
