#include "proto/timbre.pb.h"
#include "track/timbre.h"
#include "timbreutils.h"

using mixxx::track::io::timbre::TimbreModel;
using mixxx::track::io::timbre::BeatSpectrum;

double TimbreUtils::klDivergence(TimbrePointer pTimbre, TimbrePointer pTimbre2) {
    TimbreModel model1 = pTimbre->getTimbreModel();
    TimbreModel model2 = pTimbre2->getTimbreModel();

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
    return distanceGaussian(m1, v1, m2, v2);
}

double TimbreUtils::distanceGaussian(const std::vector<double> &m1,
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

