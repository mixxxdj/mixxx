#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "delayline.h"

/*
 * Gets the sample at index i in the delayline
 */

int *delayline_at(struct delayline *delayline, ptrdiff_t i)
{
    if (!delayline) {
        fprintf(stderr, "%s: Null pointer exception\n", __func__);
        return NULL;
    }

    if (delayline->current < 0)
        delayline->current += delayline->size;

    ptrdiff_t index = delayline->current + i;

    if ((size_t)index >= delayline->size)
        index -= delayline->size;

    return &delayline->array[index];
}

/*
 * Initializes the delayline
 */

void delayline_init(struct delayline *delayline)
{
    if (!delayline) {
        fprintf(stderr, "%s: Null pointer exception\n", __func__);
        return;
    }

    delayline->size = DELAYLINE_SIZE;
    delayline->current = delayline->size -1;

    for (int i = 0; i < DELAYLINE_SIZE; i++)
        delayline->array[i] = 0;
}

/*
 * Decrements the delayline pointer
 */

void delayline_decrement(struct delayline *delayline)
{
    if (!delayline) {
        fprintf(stderr, "%s: Null pointer exception\n", __func__);
        return;
    }

    delayline->current--;
    if (delayline->current < 0)
        delayline->current += delayline->size;
}

/*
 * Pushes a new sample to the delayline
 */

void delayline_push(struct delayline *delayline, int sample)
{
    if (!delayline) {
        fprintf(stderr, "%s: Null pointer exception\n", __func__);
        return;
    }

    delayline_decrement(delayline);
    delayline->array[delayline->current] = sample;
}

/*
 * Computes the average value of all samples in the delayline
 */

int delayline_avg(struct delayline *delayline)
{
    if (!delayline) {
        fprintf(stderr, "%s: Null pointer exception\n", __func__);
        return -EINVAL;
    }

    int sum = 0;

    for (int i = 0; i < delayline->size; i++)
        sum += delayline->array[i];

    return (sum / delayline->size);
}

/*
 * Prints the delayline starting at the current pointer
 */

void delayline_print(struct delayline *delayline)
{
    if (!delayline) {
        fprintf(stderr, "%s: Null pointer exception\n", __func__);
        return;
    }

    fprintf(stdout, "{");
    for (int i = 0; i < delayline->size; i++) {
        fprintf(stdout, "%d", *delayline_at(delayline, i));
        if (i < delayline->size - 1)
            fprintf(stdout, ", ");
    }
    fprintf(stdout, "}\n");
}
