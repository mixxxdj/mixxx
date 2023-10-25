#include "movinginterquartilemean.h"

MovingInterquartileMean::MovingInterquartileMean(const unsigned int listMaxSize)
        : m_dMean(0.0),
          m_iListMaxSize(listMaxSize),
          m_bChanged(true) {
}

MovingInterquartileMean::~MovingInterquartileMean() {};

double MovingInterquartileMean::insert(double value) {
    m_bChanged = true;

    // Insert new value
    if (m_list.empty()) {
        m_list.push_front(value);
        m_queue.enqueue(m_list.begin());
    } else if (value < m_list.front()) {
        m_list.push_front(value);
        m_queue.enqueue(m_list.begin());
    } else if (value >= m_list.back()) {
        m_list.push_back(value);
        m_queue.enqueue(--m_list.end());
    } else {
        std::list<double>::iterator it = m_list.begin()++;
        while (value >= *it) {
            ++it;
        }
        m_queue.enqueue(m_list.insert(it, value));
        // (If value already exists in the list, the new instance
        // is appended next to the old ones: 2·-> 1 2 3 = 1 2 2· 3)
    }

    // If list was already full, delete the oldest value:
    if (m_list.size() == static_cast<std::size_t>(m_iListMaxSize + 1)) {
        m_list.erase(m_queue.dequeue());
    }
    return mean();
}

void MovingInterquartileMean::clear() {
    m_bChanged = true;
    m_queue.clear();
    m_list.clear();
}

double MovingInterquartileMean::mean() {
    if (!m_bChanged || m_list.empty()) {
        return m_dMean;
    }

    m_bChanged = false;
    const int listSize = size();
    if (listSize <= 4) {
        double d_sum = 0;
        foreach (double d, m_list) {
            d_sum += d;
        }
        m_dMean = d_sum / listSize;
    } else if (listSize % 4 == 0) {
        int quartileSize = listSize / 4;
        double interQuartileRange = 2 * quartileSize;
        double d_sum = 0;
        std::list<double>::iterator it = m_list.begin();
        std::advance(it, quartileSize);
        for (int k = 0; k < 2 * quartileSize; ++k, ++it) {
            d_sum += *it;
        }
        m_dMean = d_sum / interQuartileRange;
    } else {
        // http://en.wikipedia.org/wiki/Interquartile_mean#Dataset_not_divisible_by_four
        double quartileSize = listSize / 4.0;
        double interQuartileRange = 2 * quartileSize;
        int nFullValues = listSize - 2 * static_cast<int>(quartileSize) - 2;
        double quartileWeight = (interQuartileRange - nFullValues) / 2;
        std::list<double>::iterator it = m_list.begin();
        std::advance(it, static_cast<int>(quartileSize));
        double d_sum = *it * quartileWeight;
        ++it;
        for (int k = 0; k < nFullValues; ++k, ++it) {
            d_sum += *it;
        }
        d_sum += *it * quartileWeight;
        m_dMean = d_sum / interQuartileRange;
    }
    return m_dMean;
}

int MovingInterquartileMean::size() const {
    return static_cast<int>(m_list.size());
}

int MovingInterquartileMean::listMaxSize() const {
    return m_iListMaxSize;
}
