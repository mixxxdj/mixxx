#include "util/indexrange.h"

#include <ostream>
#include <sstream>


namespace mixxx {

IndexRange IndexRange::splitFront(SINT startLength) {
    DEBUG_ASSERT(startLength >= 0);
    DEBUG_ASSERT(startLength <= length());
    if (start() <= end()) {
        auto startRange = forward(first, startLength);
        DEBUG_ASSERT(startRange.length() == startLength);
        first += startLength;
        DEBUG_ASSERT(start() == startRange.end()); // adjacent
        return startRange;
    } else {
        auto startRange = backward(first, startLength);
        DEBUG_ASSERT(startRange.length() == startLength);
        first -= startLength;
        DEBUG_ASSERT(start() == startRange.end()); // adjacent
        return startRange;
    }
}

IndexRange IndexRange::splitBack(SINT endLength) {
    DEBUG_ASSERT(endLength >= 0);
    DEBUG_ASSERT(endLength <= length());
    if (start() <= end()) {
        auto endRange = between(end() - endLength, end());
        DEBUG_ASSERT(endRange.length() == endLength);
        second -= endLength;
        DEBUG_ASSERT(end() == endRange.start()); // adjacent
        return endRange;
    } else {
        auto endRange = between(end() + endLength, end());
        DEBUG_ASSERT(endRange.length() == endLength);
        second += endLength;
        DEBUG_ASSERT(end() == endRange.start()); // adjacent
        return endRange;
    }
}

IndexRange reverse(IndexRange arg) {
    if (arg.empty()) {
        return arg;
    } else {
        if (arg.start() < arg.end()) {
            return IndexRange::between(arg.end() - 1, arg.start() - 1);
        } else {
            return IndexRange::between(arg.end() + 1, arg.start() + 1);
        }
    }
}

IndexRange intersect(IndexRange lhs, IndexRange rhs) {
    if (lhs.start() <= lhs.end()) {
        if (rhs.start() <= rhs.end()) {
            const SINT start = std::max(lhs.start(), rhs.start());
            const SINT end = std::min(lhs.end(), rhs.end());
            if (start <= end) {
                return IndexRange::between(start, end);
            }
        } else {
            DEBUG_ASSERT(!"Cannot intersect index ranges with contrary orientations");
        }
    } else {
        if (rhs.start() >= rhs.end()) {
            const SINT start = std::min(lhs.start(), rhs.start());
            const SINT end = std::max(lhs.end(), rhs.end());
            if (start >= end) {
                return IndexRange::between(start, end);
            }
        } else {
            DEBUG_ASSERT(!"Cannot intersect index ranges with contrary orientations");
        }
    }
    return IndexRange();
}

IndexRange span(IndexRange lhs, IndexRange rhs) {
    if (lhs.start() <= lhs.end()) {
        if (rhs.start() <= rhs.end()) {
            const SINT start = std::min(lhs.start(), rhs.start());
            const SINT end = std::max(lhs.end(), rhs.end());
            DEBUG_ASSERT(start <= end);
            return IndexRange::between(start, end);
        } else {
            DEBUG_ASSERT(!"Cannot span index ranges with contrary orientations");
        }
    } else {
        if (rhs.start() >= rhs.end()) {
            const SINT start = std::max(lhs.start(), rhs.start());
            const SINT end = std::min(lhs.end(), rhs.end());
            DEBUG_ASSERT(start >= end);
            return IndexRange::between(start, end);
        } else {
            DEBUG_ASSERT(!"Cannot span index ranges with contrary orientations");
        }
    }
    return IndexRange();
}

std::ostream& operator<<(std::ostream& os, IndexRange arg) {
    return os << '[' << arg.start() << " -> " << arg.end() << ')';
}

QDebug operator<<(QDebug dbg, IndexRange arg) {
    std::ostringstream oss;
    oss << arg;
    return dbg << oss.str().c_str();
}

} // namespace mixxx
