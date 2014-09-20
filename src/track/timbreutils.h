#ifndef TIMBREUTILS_H
#define TIMBREUTILS_H

#include "track/timbre.h"

class TimbreUtils {
  public:
    static double klDivergence(TimbrePointer pTimbre,
        TimbrePointer pTimbre2);
    static double symmetricKlDivergence(TimbrePointer pTimbre,
        TimbrePointer pTimbre2);
    static double hellingerDistance(TimbrePointer pTimbre,
        TimbrePointer pTimbre2);
    static double modelDistanceBeats(TimbrePointer pTimbre,
        TimbrePointer pTimbre2);

private:
    typedef double (*DistanceFunc)(const std::vector<double> &, //mean 1
        const std::vector<double> &, //variance 1
        const std::vector<double> &, //mean 2
        const std::vector<double> &); //variance 2

    static double modelDistance(TimbrePointer pTimbre,
        TimbrePointer pTimbre2,
        DistanceFunc distanceFunc);

    static double distanceKL(const std::vector<double> &m1,
        const std::vector<double> &v1, const std::vector<double> &m2,
        const std::vector<double> &v2);

    static double distanceSymmetricKL(const std::vector<double> &m1,
            const std::vector<double> &v1, const std::vector<double> &m2,
            const std::vector<double> &v2);

    static double distanceHellinger(const std::vector<double> &m1,
        const std::vector<double> &v1, const std::vector<double> &m2,
        const std::vector<double> &v2);

    static double distanceCosine(const std::vector<double> &v1,
        const std::vector<double> &v2);
};
#endif // TIMBREUTILS_H
