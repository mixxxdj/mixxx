#ifndef FILTERS_H

#define FILTERS_H

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

#endif /* end of include guard FILTERS_H */
