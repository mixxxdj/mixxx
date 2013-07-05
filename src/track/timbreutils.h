#ifndef TIMBREUTILS_H
#define TIMBREUTILS_H

#include "track/timbre.h"

class TimbreUtils {
  public:
    static double klDivergence(TimbrePointer pTimbre, TimbrePointer pTimbre2);
  private:
    static double distanceGaussian(const std::vector<double> &m1,
                                   const std::vector<double> &v1,
                                   const std::vector<double> &m2,
                                   const std::vector<double> &v2);
};

#endif // TIMBREUTILS_H
