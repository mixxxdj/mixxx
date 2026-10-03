#pragma once

#include <chrono>
#include <utility>
#include <vector>

// HostTimeFilter is a class that provides a robust, jitter-free host time
// for time points of an auxiliary clock using linear regression.
class HostTimeFilter {
  public:
    static constexpr std::chrono::microseconds kInvalidHostTime = std::chrono::microseconds::min();

    explicit HostTimeFilter(const std::size_t numPoints)
            : m_numPoints(numPoints),
              m_index(0),
              m_sumAux(0.0),
              m_sumHst(0.0),
              m_sumAuxByHst(0.0),
              m_sumAuxSquared(0.0) {
        m_points.reserve(m_numPoints);
    }

    void clear() {
        m_index = 0;
        m_points.clear();
        m_sumAux = 0.0;
        m_sumHst = 0.0;
        m_sumAuxByHst = 0.0;
        m_sumAuxSquared = 0.0;
    }

    // Inserts a new time point consisting of an auxiliary time and a host time.
    // These points are used later to calculate the filtered host time using linear regression.
    void insertTimePoint(
            double auxiliaryTime, std::chrono::microseconds hostTime) {
        const auto micros = hostTime.count();
        const auto timePoint = std::make_pair(auxiliaryTime, static_cast<double>(micros));

        if (m_points.size() < m_numPoints) {
            m_points.push_back(timePoint);
            m_sumAux += timePoint.first;
            m_sumHst += timePoint.second;
            m_sumAuxByHst += timePoint.first * timePoint.second;
            m_sumAuxSquared += timePoint.first * timePoint.first;
        } else {
            const auto& prevPoint = m_points[m_index];
            m_sumAux += timePoint.first - prevPoint.first;
            m_sumHst += timePoint.second - prevPoint.second;
            m_sumAuxByHst += timePoint.first * timePoint.second -
                    prevPoint.first * prevPoint.second;
            m_sumAuxSquared += timePoint.first * timePoint.first -
                    prevPoint.first * prevPoint.first;
            m_points[m_index] = timePoint;
        }
        m_index = (m_index + 1) % m_numPoints;
    }

    // Calculates the host time based on the auxiliary time using linear regression.
    // Returns kInvalidHostTime if there are not enough points or if the calculation isn't possible.
    std::chrono::microseconds calcHostTime(double auxiliaryTime) const {
        if (m_points.size() < 2) {
            return kInvalidHostTime;
        }

        const double n = static_cast<double>(m_points.size());
        const double denominator = (n * m_sumAuxSquared - m_sumAux * m_sumAux);
        if (denominator == 0.0) {
            return kInvalidHostTime;
        }

        const double slope = (n * m_sumAuxByHst - m_sumAux * m_sumHst) / denominator;
        const double intercept = (m_sumHst - slope * m_sumAux) / n;

        return std::chrono::microseconds(
                static_cast<long long>(slope * auxiliaryTime + intercept));
    }

  private:
    const std::size_t m_numPoints;
    std::size_t m_index;
    std::vector<std::pair<double, double>> m_points;
    double m_sumAux;
    double m_sumHst;
    double m_sumAuxByHst;
    double m_sumAuxSquared;
};
