#ifndef DELAYLINE_H

#define DELAYLINE_H

#include <stddef.h>

#define DELAYLINE_SIZE 5

struct delayline {
    size_t size;
    int array[DELAYLINE_SIZE];
    ptrdiff_t current;
};

void delayline_init(struct delayline *delayline);
int *delayline_at(struct delayline *delayline, ptrdiff_t i);
void delayline_push(struct delayline *delayline, int sample);
int delayline_avg(struct delayline *delayline);
void delayline_print(struct delayline *delayline);

#endif /* end of include guard DELAYLINE_H */
