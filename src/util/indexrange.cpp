#include "util/indexrange.h"

#include <ostream>
#include <sstream>


namespace mixxx {

IndexRange IndexRange::splitHead(SINT headLength) {
    DEBUG_ASSERT(headLength >= 0);
    DEBUG_ASSERT(headLength <= length());
    if (head() <= tail()) {
        auto headRange = forward(first, headLength);
        DEBUG_ASSERT(headRange.length() == headLength);
        first += headLength;
        DEBUG_ASSERT(head() == headRange.tail()); // adjacent
        return headRange;
    } else {
        auto headRange = backward(first, headLength);
        DEBUG_ASSERT(headRange.length() == headLength);
        first -= headLength;
        DEBUG_ASSERT(head() == headRange.tail()); // adjacent
        return headRange;
    }
}

IndexRange IndexRange::splitTail(SINT tailLength) {
    DEBUG_ASSERT(tailLength >= 0);
    DEBUG_ASSERT(tailLength <= length());
    if (head() <= tail()) {
        auto tailRange = between(tail() - tailLength, tail());
        DEBUG_ASSERT(tailRange.length() == tailLength);
        second -= tailLength;
        DEBUG_ASSERT(tail() == tailRange.head()); // adjacent
        return tailRange;
    } else {
        auto tailRange = between(tail() + tailLength, tail());
        DEBUG_ASSERT(tailRange.length() == tailLength);
        second += tailLength;
        DEBUG_ASSERT(tail() == tailRange.head()); // adjacent
        return tailRange;
    }
}

IndexRange reverse(IndexRange arg) {
    if (arg.empty()) {
        return arg;
    } else {
        if (arg.head() < arg.tail()) {
            return IndexRange::between(arg.tail() - 1, arg.head() - 1);
        } else {
            return IndexRange::between(arg.tail() + 1, arg.head() + 1);
        }
    }
}

IndexRange intersect(IndexRange lhs, IndexRange rhs) {
    if (lhs.head() <= lhs.tail()) {
        if (rhs.head() <= rhs.tail()) {
            const SINT head = std::max(lhs.head(), rhs.head());
            const SINT tail = std::min(lhs.tail(), rhs.tail());
            if (head <= tail) {
                return IndexRange::between(head, tail);
            }
        } else {
            DEBUG_ASSERT(!"Cannot intersect index ranges with contrary orientations");
        }
    } else {
        if (rhs.head() >= rhs.tail()) {
            const SINT head = std::min(lhs.head(), rhs.head());
            const SINT tail = std::max(lhs.tail(), rhs.tail());
            if (head >= tail) {
                return IndexRange::between(head, tail);
            }
        } else {
            DEBUG_ASSERT(!"Cannot intersect index ranges with contrary orientations");
        }
    }
    return IndexRange();
}

IndexRange join(IndexRange lhs, IndexRange rhs) {
    if (lhs.head() <= lhs.tail()) {
        if (rhs.head() <= rhs.tail()) {
            const SINT head = std::min(lhs.head(), rhs.head());
            const SINT tail = std::max(lhs.tail(), rhs.tail());
            DEBUG_ASSERT(head <= tail);
            return IndexRange::between(head, tail);
        } else {
            DEBUG_ASSERT(!"Cannot join index ranges with contrary orientations");
        }
    } else {
        if (rhs.head() >= rhs.tail()) {
            const SINT head = std::max(lhs.head(), rhs.head());
            const SINT tail = std::min(lhs.tail(), rhs.tail());
            DEBUG_ASSERT(head >= tail);
            return IndexRange::between(head, tail);
        } else {
            DEBUG_ASSERT(!"Cannot join index ranges with contrary orientations");
        }
    }
    return IndexRange();
}

std::ostream& operator<<(std::ostream& os, IndexRange arg) {
    return os << '[' << arg.head() << " -> " << arg.tail() << ')';
}

QDebug operator<<(QDebug dbg, IndexRange arg) {
    std::ostringstream oss;
    oss << arg;
    return dbg << oss.str().c_str();
}

} // namespace mixxx
