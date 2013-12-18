#include <math.h>
#include <QDebug>
#include "proto/timbre.pb.h"
#include "track/timbre.h"
#include "timbreutils.h"

using mixxx::track::io::timbre::TimbreModel;
using mixxx::track::io::timbre::BeatSpectrum;

double TimbreUtils::klDivergence(TimbrePointer pTimbre,
        TimbrePointer pTimbre2) {
    return modelDistance(pTimbre, pTimbre2, distanceKL);
}

double TimbreUtils::symmetricKlDivergence(TimbrePointer pTimbre,
        TimbrePointer pTimbre2) {
    return modelDistance(pTimbre, pTimbre2, distanceSymmetricKL);
}

double TimbreUtils::hellingerDistance(TimbrePointer pTimbre,
        TimbrePointer pTimbre2) {
    return modelDistance(pTimbre, pTimbre2, distanceHellinger);
}

double TimbreUtils::modelDistance(TimbrePointer pTimbre,
        TimbrePointer pTimbre2, DistanceFunc distanceFunc) {
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

double TimbreUtils::modelDistanceBeats(TimbrePointer pTimbre,
        TimbrePointer pTimbre2) {
    const TimbreModel& model1 = pTimbre->getTimbreModel();
    const TimbreModel& model2 = pTimbre2->getTimbreModel();

    if (model1.has_beat_spectrum() && model2.has_beat_spectrum()) {
        const BeatSpectrum& beat1 = model1.beat_spectrum();
        int beat1_size = beat1.feature_size();
        std::vector<double> b1(beat1_size);
        for (int i = 0; i < beat1_size; i++) {
            b1[i] = beat1.feature(i);
        }

        const BeatSpectrum& beat2 = model2.beat_spectrum();
        int beat2_size = beat2.feature_size();
        std::vector<double> b2(beat2_size);
        for (int i = 0; i < beat2_size; i++) {
            b2[i] = beat2.feature(i);
        }
        return distanceCosine(b1, b2);
    }
    return 1.0;
}

double TimbreUtils::distanceKL(const std::vector<double> &m1,
        const std::vector<double> &v1, const std::vector<double> &m2,
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

double TimbreUtils::distanceSymmetricKL(const std::vector<double> &m1,
        const std::vector<double> &v1, const std::vector<double> &m2,
        const std::vector<double> &v2) {
    int dim = m1.size();
    double small = 1e-20;
    double cov_term = 0.0;
    double mean_term = 0.0;

    for (int k = 0; k < dim; k++) {
        double pv = v1[k] + small;
        double qv = v2[k] + small;
        double inv_pv = 1.0 / pv;
        double inv_qv = 1.0 / qv;
        cov_term += (pv * inv_qv) + (qv * inv_pv);
        double diff_mean = (m1[k] - m2[k]) + small;
        mean_term += diff_mean * (inv_pv + inv_qv) * diff_mean;
    }

    return (0.5 * (cov_term + mean_term)) - dim;
}

double TimbreUtils::distanceHellinger(const std::vector<double> &m1,
        const std::vector<double> &v1, const std::vector<double> &m2,
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

double TimbreUtils::distanceCosine(const std::vector<double> &v1,
        const std::vector<double> &v2) {
    // copied from QM DSP library -- CosineDistance.cpp
    double dist = 1.0;
    double dDenTot = 0;
    double dDen1 = 0;
    double dDen2 = 0;
    double dSum1 = 0;
    double small = 1e-20;

    //check if v1, v2 same size
    if (v1.size() != v2.size()) {
        qDebug() << "TimbreUtils::distanceCosine:ERROR: vectors not the same size.";
        return 1.0;
    } else {
        for (int i=0; i < v1.size(); i++) {
            dSum1 += v1[i]*v2[i];
            dDen1 += v1[i]*v1[i];
            dDen2 += v2[i]*v2[i];
        }
        dDenTot = sqrt(abs(dDen1*dDen2)) + small;
        dist = 1-((dSum1)/dDenTot);
        return dist;
    }
}
