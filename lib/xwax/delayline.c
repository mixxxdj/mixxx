#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "delayline.h"

int *delayline_at(struct delayline *delayline, ptrdiff_t i)
{
    if (!delayline) {
        printf("%s: Null pointer exception\n", __func__);
        return NULL;
    }

    if (delayline->current < 0)
        delayline->current += delayline->size;

    ptrdiff_t index = delayline->current + i;

    if ((size_t)index >= delayline->size)
        index -= delayline->size;

    return &delayline->array[index];
}

void delayline_init(struct delayline *delayline)
{   
	if (!delayline) {
        printf("%s: Null pointer exception\n", __func__);
        return;
    }

        delayline->size = DELAYLINE_SIZE;
		delayline->current = delayline->size -1;
        for (int i = 0; i < DELAYLINE_SIZE; i++)
            delayline->array[i] = 0;
}

void delayline_decrement(struct delayline *delayline)
{
    if (!delayline) {
        printf("%s: Null pointer exception\n", __func__);
        return;
    }

    delayline->current--;
    if (delayline->current < 0)
        delayline->current += delayline->size;
}

void delayline_push(struct delayline *delayline, int sample)
{
    if (!delayline) {
        printf("%s: Null pointer exception\n", __func__);
        return;
    }

    delayline_decrement(delayline);
    delayline->array[delayline->current] = sample;
}

int delayline_avg(struct delayline *delayline)
{
    if (!delayline) {
        printf("%s: Null pointer exception\n", __func__);
        return -EINVAL;
    }

    int sum = 0;

    for (int i = 0; i < delayline->size; i++)
        sum += delayline->array[i];

    return (sum / delayline->size);
}

/* Prints the circular buffer starting at the current read pointer  */
void delayline_print(struct delayline *delayline)
{
    if (!delayline) {
        printf("%s: Null pointer exception\n", __func__);
        return;
    }

    printf("{");
    for (int i = 0; i < delayline->size; i++) {
        printf("%d", *delayline_at(delayline, i));
        if (i < delayline->size - 1)
            printf(", ");
    }
    printf("}\n");
}
