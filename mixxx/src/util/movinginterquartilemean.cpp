#include "movinginterquartilemean.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>

#include "util/assert.h"

double MovingInterquartileMean::insert(double value) {
    // make space if needed
    // NOTE: after benchmarking, replacing the erase+insert with a rotate+swap does
    // not result in significant enough speedup to warrant the complexity.
    if (m_list.size() == m_list.capacity()) {
        m_list.erase(std::lower_bound(m_list.begin(), m_list.end(), m_queue.front()));
        m_queue.pop();
    }
    auto insertPosition = std::lower_bound(m_list.cbegin(), m_list.cend(), value);
    m_list.insert(insertPosition, value);
    // we explicitly insert the value and not an index or iterator here,
    // because those would get invalidated when the contents of m_list are
    // shifted around (due to the erase and insert above). updating those
    // iterators/indices is likely more expensive than recovering them when
    // needed using the first std::lower_bound
    m_queue.push(value);

    DEBUG_ASSERT(std::is_sorted(m_list.cbegin(), m_list.cend()));

    // no need to set m_bChanged and check m_list.empty().
    // we know the preconditions are satisfied so call `calcMean()` directly
    m_dMean = calcMean();
    return m_dMean;
}

void MovingInterquartileMean::clear() {
    m_bChanged = true;
    // std::queue has no .clear(), so creating a temporary and std::swap is the
    // next most elegant solution
    std::queue<double>().swap(m_queue);
    m_list.clear();
}

double MovingInterquartileMean::mean() {
    if (!m_bChanged || m_list.empty()) {
        return m_dMean;
    }
    m_dMean = calcMean();
    m_bChanged = false;
    return m_dMean;
}

double MovingInterquartileMean::calcMean() const {
    // assumes m_list is not empty
    auto simpleMean = [](auto begin, auto end) -> double {
        double size = std::distance(begin, end);
        return std::accumulate(begin, end, 0.0) / size;
    };

    const auto listSize = m_list.size();
    if (listSize <= 4) {
        return simpleMean(m_list.cbegin(), m_list.cend());
    } else if (listSize % 4 == 0) {
        std::size_t quartileSize = listSize / 4;
        auto start = m_list.cbegin() + quartileSize;
        auto end = m_list.cend() - quartileSize;
        return simpleMean(start, end);
    } else {
        // http://en.wikipedia.org/wiki/Interquartile_mean#Dataset_not_divisible_by_four
        double quartileSize = listSize / 4.0;
        double interQuartileRange = 2 * quartileSize;
        std::size_t nFullValues = listSize - 2 * static_cast<std::size_t>(quartileSize) - 2;
        double quartileWeight = (interQuartileRange - nFullValues) / 2;
        auto it = m_list.begin();
        std::advance(it, static_cast<std::size_t>(quartileSize));
        double d_sum = *it * quartileWeight;
        ++it;
        for (std::size_t k = 0; k < nFullValues; ++k, ++it) {
            d_sum += *it;
        }
        d_sum += *it * quartileWeight;
        return d_sum / interQuartileRange;
    }
}
