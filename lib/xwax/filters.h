#ifndef FILTERS_H

#define FILTERS_H

int ema(const int x, int *ema_old, const double alpha);
int derivative(const int x, int *x_old);
int rms(int x, unsigned long long *rms_old);

#endif /* end of include guard FILTERS_H */

