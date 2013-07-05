#ifndef TIMBREUTILS_H
#define TIMBREUTILS_H

#include "track/timbre.h"

class TimbreUtils {
  public:
    static double klDivergence(TimbrePointer pTimbre, TimbrePointer pTimbre2) const;
  private:
    static double distanceGaussian() const;
};

#endif // TIMBREUTILS_H
