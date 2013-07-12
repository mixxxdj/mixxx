#include <math.h>
#include "proto/timbre.pb.h"
#include "track/timbre.h"
#include "timbreutils.h"

using mixxx::track::io::timbre::TimbreModel;
using mixxx::track::io::timbre::BeatSpectrum;

double TimbreUtils::klDivergence(TimbrePointer pTimbre,
                                  TimbrePointer pTimbre2) {
    return modelDistance(pTimbre, pTimbre2, distanceKL);
}

double TimbreUtils::hellingerDistance(TimbrePointer pTimbre,
                                  TimbrePointer pTimbre2) {
    return modelDistance(pTimbre, pTimbre2, distanceHellinger);
}

double TimbreUtils::modelDistance(TimbrePointer pTimbre,
                                  TimbrePointer pTimbre2,
                                  DistanceFunc distanceFunc) {
    const TimbreModel& model1 = pTimbre->getTimbreModel();
    const TimbreModel& model2 = pTimbre2->getTimbreModel();

    int m_size = model1.mean_size();

    std::vector<double> m1(m_size);
    std::vector<double> v1(m_size);
    std::vector<double> m2(m_size);
    std::vector<double> v2(m_size);

    for (int i = 0; i < m_size; i++) {
        m1[i] = model1.mean(i);
        v1[i] = model1.variance(i);
        m2[i] = model2.mean(i);
        v2[i] = model2.variance(i);
    }
    return distanceFunc(m1, v1, m2, v2);
}

double TimbreUtils::distanceKL(const std::vector<double> &m1,
                               const std::vector<double> &v1,
                               const std::vector<double> &m2,
                               const std::vector<double> &v2) {
    // copied from QM DSP library - KLDivergence.cpp
    int sz = m1.size();

    double d = -2.0 * sz;
    double small = 1e-20;

    for (int k = 0; k < sz; ++k) {

        double kv1 = v1[k] + small;
        double kv2 = v2[k] + small;
        double km = (m1[k] - m2[k]) + small;

        d += kv1 / kv2 + kv2 / kv1;
        d += km * (1.0 / kv1 + 1.0 / kv2) * km;
    }

    d /= 2.0;

    return d;
}

double TimbreUtils::distanceHellinger(const std::vector<double> &m1,
                                      const std::vector<double> &v1,
                                      const std::vector<double> &m2,
                                      const std::vector<double> &v2) {
    int dim = m1.size();

    double small = 1e-20;

    double det_p = 1.0;
    double det_q = 1.0;
    double det_combined = 1.0;

    double exp_term = 0.0;

    for (int k = 0; k < dim; k++) {
        double pv = v1[k] + small;
        double qv = v2[k] + small;
        det_p *= pv;
        det_q *= qv;
        double combined_v = 0.5 * (pv + qv);
        det_combined *= combined_v;

        double diff_mean = (m1[k] - m2[k]) + small;

        exp_term += diff_mean * (1.0/combined_v) * diff_mean;
    }

    double h2 = 1 - ((sqrt((sqrt(det_p)*sqrt(det_q)))/sqrt(det_combined)) *
        exp(-0.125 * exp_term));
    return sqrt(h2);
}
