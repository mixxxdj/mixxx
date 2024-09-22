#pragma once

#include <QQueue>

// Simple moving average
class Rotary {
  public:
    Rotary(qsizetype filterLength)
            : m_filterHistory(QList<double>(filterLength, 0.0)){};
    // Low pass filtered rotary event
    double filter(double value);

  private:
    QQueue<double> m_filterHistory;
};
