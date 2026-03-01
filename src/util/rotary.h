#pragma once

#include <vector>

#include "util/assert.h"

// Simple moving average
class Rotary {
    using Buffer = std::vector<double>;
    using index_type = Buffer::size_type;

  public:
    Rotary(qsizetype filterLength)
            : m_filterHistory(filterLength, 0.0),
              m_headIndex{0} {
        DEBUG_ASSERT(filterLength > 0);
    };
    // Low pass filtered rotary event
    double filter(double value);

  private:
    index_type nextIndex(index_type) const;
    void append(double v);
    double calculate() const;
    Buffer m_filterHistory;
    index_type m_headIndex;
};
