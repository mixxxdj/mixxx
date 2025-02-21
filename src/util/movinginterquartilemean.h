#pragma once

#include <queue>
#include <vector>

// Truncated Interquartile mean

// TruncatedIQM keeps an ordered list with the last n (_capacity_)
// input doubles and calculates the mean discarding the lowest 25%
// and the highest 25% values in order to reduce sensitivity to outliers.
//
// http://en.wikipedia.org/wiki/Interquartile_mean
class MovingInterquartileMean {
  public:
    // Constructs an empty MovingTruncatedIQM.
    MovingInterquartileMean(std::size_t listLength)
            : m_dMean(0.0),
              m_bChanged(true) {
        m_list.reserve(listLength);
    }

    // Inserts value to the list and returns the new truncated mean.
    double insert(double value);
    // Empty the list.
    void clear();
    // Returns the current truncated mean. Input list must not be empty.
    double mean();
    // Returns how many values have been input.
    int size() const {
        return static_cast<int>(m_list.size());
    }

  private:
    double calcMean() const;
    // The list keeps input doubles ordered by value.
    std::vector<double> m_list;
    // The queue keeps a second copy of the list, but in insertion
    // order. This is to track which value we need to evict in order
    // not stay within memory constraints.
    std::queue<double> m_queue;
    double m_dMean;

    // sum() checks this to know if it has to recalculate the mean.
    bool m_bChanged;
};
