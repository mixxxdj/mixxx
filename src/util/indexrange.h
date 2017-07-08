#ifndef MIXXX_INDEXRANGE_H
#define MIXXX_INDEXRANGE_H

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

    static IndexRange between(SINT head, SINT tail) {
        return IndexRange(head, tail);
    }
    static IndexRange forward(SINT head, SINT length) {
        DEBUG_ASSERT(length >= 0);
        return IndexRange(head, head + length);
    }
    static IndexRange backward(SINT head, SINT length) {
        DEBUG_ASSERT(length >= 0);
        return IndexRange(head, head - length);
    }

    // The near boundary (inclusive)
    SINT head() const {
        return first;
    }
    // The opposite boundary (exclusive)
    SINT tail() const {
        return second;
    }

    bool empty() const {
        return head() == tail();
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
            if (head() < tail()) {
                return Orientation::Forward;
            } else {
                return Orientation::Backward;
            }
        }
    }

    SINT length() const {
        return (head() <= tail()) ? (tail() - head()) : (head() - tail());
    }

    // Clamps index by this range including both head() and tail()
    // boundaries.
    SINT clamp(SINT index) const {
        if (head() <= tail()) {
            return std::max(head(), std::min(tail(), index));
        } else {
            return std::min(head(), std::max(tail(), index));
        }
    }

    bool contains(SINT index) const {
        if (head() <= tail()) {
            return (head() <= index) && (index < tail());
        } else {
            return (head() >= index) && (index > tail());
        }
    }

    // Splits this range into two adjacent parts by slicing off
    // and returning a range of given length and same direction
    // from the head side. The given head length must not exceed
    // the length of this range.
    IndexRange splitHead(SINT headLength);

    // Splits this range into two adjacent parts by slicing off
    // and returning a range of given length and same direction
    // from the tail side. The given tail length must not exceed
    // the length of this range.
    IndexRange splitTail(SINT tailLength);

    // Same as splitHead(), but omitting the return value.
    void dropHead(SINT headLength) {
        DEBUG_ASSERT(headLength >= 0);
        DEBUG_ASSERT(headLength <= length());
        if (head() <= tail()) {
            first += headLength;
        } else {
            first -= headLength;
        }
    }

    // Same as splitTail(), but omitting the return value.
    void dropTail(SINT tailLength) {
        DEBUG_ASSERT(tailLength >= 0);
        DEBUG_ASSERT(tailLength <= length());
        if (head() <= tail()) {
            second -= tailLength;
        } else {
            second += tailLength;
        }
    }

    friend
    bool operator==(IndexRange lhs, IndexRange rhs) {
        return (lhs.first == rhs.first) && (lhs.second == rhs.second);
    }

  private:
    IndexRange(SINT head, SINT tail)
        : Super(head, tail) {
    }
};

IndexRange reverse(IndexRange arg);

IndexRange intersect(IndexRange lhs, IndexRange rhs);

IndexRange join(IndexRange lhs, IndexRange rhs);

inline
bool operator!=(IndexRange lhs, IndexRange rhs) {
    return !(lhs == rhs);
}

inline
bool operator<=(IndexRange lhs, IndexRange rhs) {
    return lhs.empty() || (intersect(lhs, rhs) == lhs);
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


#endif // MIXXX_INDEXRANGE_H
