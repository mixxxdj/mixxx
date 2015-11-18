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
    if (m_list.isEmpty()) {
        m_list.prepend(value);
        m_queue.enqueue(m_list.begin());
    } else if (value < m_list.first()) {
        m_list.prepend(value);
        m_queue.enqueue(m_list.begin());
    } else if (value >= m_list.last()) {
        m_list.append(value);
        m_queue.enqueue(--m_list.end());
    } else {
        QLinkedList<double>::iterator it = m_list.begin()++;
        while (value >= *it) {
            ++it;
        }
        m_queue.enqueue(m_list.insert(it, value));
        // (If value already exists in the list, the new instance
        // is appended next to the old ones: 2·-> 1 2 3 = 1 2 2· 3)
    }

    // If list was already full, delete the oldest value:
    if (m_list.size() == m_iListMaxSize + 1) {
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
    if (m_list.size() <= 4) {
        double d_sum = 0;
        foreach (double d, m_list) {
            d_sum += d;
        }
        m_dMean = d_sum / m_list.size();
    } else if (m_list.size() % 4 == 0) {
        int quartileSize = m_list.size() / 4;
        double interQuartileRange = 2 * quartileSize;
        double d_sum = 0;
        QLinkedList<double>::iterator it = m_list.begin() + quartileSize;
        for (int k = 0; k < 2 * quartileSize; ++k, ++it) {
            d_sum += *it;
        }
        m_dMean = d_sum / interQuartileRange;
    } else {
        // http://en.wikipedia.org/wiki/Interquartile_mean#Dataset_not_divisible_by_four
        double quartileSize = m_list.size() / 4.0;
        double interQuartileRange = 2 * quartileSize;
        int nFullValues = m_list.size() - 2*static_cast<int>(quartileSize) - 2;
        double quartileWeight = (interQuartileRange - nFullValues) / 2;
        QLinkedList<double>::iterator it = m_list.begin() + static_cast<int>(quartileSize);
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
    return m_list.size();
}

int MovingInterquartileMean::listMaxSize() const {
    return m_iListMaxSize;
}
