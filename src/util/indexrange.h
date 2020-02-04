#pragma once


#include <QtDebug>

#include <iosfwd>
#include <utility>

#include "util/assert.h"
#include "util/types.h"


namespace mixxx {

// A half-open, directed range of indices with limited mutability.
class IndexRange final: private std::pair<SINT, SINT> {
    typedef std::pair<SINT, SINT> Super;

  public:
    using Super::swap;

    IndexRange()
        : Super(0, 0) {
        DEBUG_ASSERT(empty());
    }

    static IndexRange between(SINT start, SINT end) {
        return IndexRange(start, end);
    }
    static IndexRange forward(SINT start, SINT length) {
        DEBUG_ASSERT(length >= 0);
        return IndexRange(start, start + length);
    }
    static IndexRange backward(SINT start, SINT length) {
        DEBUG_ASSERT(length >= 0);
        return IndexRange(start, start - length);
    }

    // The first index within this range (inclusive)
    SINT start() const {
        return first;
    }
    // The next index beyond this range (exclusive)
    SINT end() const {
        return second;
    }

    bool empty() const {
        return start() == end();
    }

    enum class Orientation {
        Empty,
        Forward,
        Backward,
    };
    Orientation orientation() const {
        if (empty()) {
            return Orientation::Empty; // undefined
        } else {
            if (start() < end()) {
                return Orientation::Forward;
            } else {
                return Orientation::Backward;
            }
        }
    }

    SINT length() const {
        return (start() <= end()) ? (end() - start()) : (start() - end());
    }

    // Clamps index by this range including both start() and end()
    // boundaries.
    SINT clampIndex(SINT index) const {
        if (start() <= end()) {
            return std::max(start(), std::min(end(), index));
        } else {
            return std::min(start(), std::max(end(), index));
        }
    }

    bool containsIndex(SINT index) const {
        if (start() <= end()) {
            return (start() <= index) && (index < end());
        } else {
            return (start() >= index) && (index > end());
        }
    }

    // Grow the range by appending the given length at the front side.
    void growFront(SINT frontLength) {
        DEBUG_ASSERT(frontLength >= 0);
        if (start() <= end()) {
            first -= frontLength;
        } else {
            first += frontLength;
        }
    }

    // Grow the range by appending the given length at the back side.
    void growBack(SINT backLength) {
        DEBUG_ASSERT(backLength >= 0);
        if (start() <= end()) {
            second += backLength;
        } else {
            second -= backLength;
        }
    }

    // Shrink the range by cutting off the given length at the front side.
    void shrinkFront(SINT frontLength) {
        DEBUG_ASSERT(frontLength >= 0);
        DEBUG_ASSERT(frontLength <= length());
        if (start() <= end()) {
            first += frontLength;
        } else {
            first -= frontLength;
        }
    }

    // Shrink the range by cutting off the given length at the back side.
    void shrinkBack(SINT backLength) {
        DEBUG_ASSERT(backLength >= 0);
        DEBUG_ASSERT(backLength <= length());
        if (start() <= end()) {
            second -= backLength;
        } else {
            second += backLength;
        }
    }

    // Splits this range into two adjacent parts by slicing off
    // and returning a range of given length and same direction
    // from the front side. The given front length must not exceed
    // the length of this range.
    IndexRange splitAndShrinkFront(SINT frontLength);

    // Splits this range into two adjacent parts by slicing off
    // and returning a range of given length and same direction
    // from the back side. The given back length must not exceed
    // the length of this range.
    IndexRange splitAndShrinkBack(SINT backLength);

    friend
    bool operator==(IndexRange lhs, IndexRange rhs) {
        return (lhs.first == rhs.first) && (lhs.second == rhs.second);
    }

  private:
    IndexRange(SINT start, SINT end)
        : Super(start, end) {
    }
};

IndexRange reverse(IndexRange arg);

IndexRange intersect(IndexRange lhs, IndexRange rhs);

IndexRange span(IndexRange lhs, IndexRange rhs);

inline
bool operator!=(IndexRange lhs, IndexRange rhs) {
    return !(lhs == rhs);
}

inline
bool operator<=(IndexRange lhs, IndexRange rhs) {
    return intersect(lhs, rhs) == lhs;
}

inline
bool operator>=(IndexRange lhs, IndexRange rhs) {
    return rhs <= lhs;
}

inline
bool operator<(IndexRange lhs, IndexRange rhs) {
    return (lhs.length() < rhs.length()) && (lhs <= rhs);
}

inline
bool operator>(IndexRange lhs, IndexRange rhs) {
    return rhs < lhs;
}

std::ostream& operator<<(std::ostream& os, IndexRange arg);

QDebug operator<<(QDebug dbg, IndexRange arg);


} // namespace mixxx
