#ifndef FILTERS_H

#define FILTERS_H

#include "ringbuffer.h"

struct ewma_filter {
    double alpha;
    int y_old;
};

void ewma_init(struct ewma_filter *f, const double alpha);
int ewma(struct ewma_filter *f, const int x);

struct differentiator {
    int x_old;
};

void derivative_init(struct differentiator *f);
int derivative(struct differentiator *f, const int x);

struct root_mean_square {
    float alpha;
    unsigned long long squared_old;
};

void rms_init(struct root_mean_square *f, const float alpha);
int rms(struct root_mean_square *f, const int x);

struct savitzky_golay {
    size_t window_size;           /* Window size */
    size_t M;                     /* Half width */
    size_t polyorder;             /* Polynomial order */
    double *coeff;                /* Filter coefficeints */
    struct ringbuffer *delayline; /* Sample ringbuffer */
};

struct savitzky_golay *savgol_create(size_t window_size, size_t polyorder);
void savgol_destroy(struct savitzky_golay *f);
int savgol(struct savitzky_golay *f, int x);

struct rumble_filter {
    double fc;    /* Cutoff frequency */
    double fs;    /* Sample rate */
    double alpha; /* Filter coefficient */
    int x_old;    /* x[n-1] */
    int y_old;    /* y[n-1] */
};

void rhpf_init(struct rumble_filter *f, unsigned int fs, double fc);
int rhpf_process(struct rumble_filter *f, int x);

#endif /* end of include guard FILTERS_H */
