#include "util/indexrange.h"

#include <ostream>
#include <sstream>


namespace mixxx {

IndexRange IndexRange::splitAndShrinkFront(SINT frontLength) {
    DEBUG_ASSERT(frontLength >= 0);
    DEBUG_ASSERT(frontLength <= length());
    if (start() <= end()) {
        auto startRange = forward(first, frontLength);
        DEBUG_ASSERT(startRange.length() == frontLength);
        first += frontLength;
        DEBUG_ASSERT(start() == startRange.end()); // adjacent
        return startRange;
    } else {
        auto startRange = backward(first, frontLength);
        DEBUG_ASSERT(startRange.length() == frontLength);
        first -= frontLength;
        DEBUG_ASSERT(start() == startRange.end()); // adjacent
        return startRange;
    }
}

IndexRange IndexRange::splitAndShrinkBack(SINT backLength) {
    DEBUG_ASSERT(backLength >= 0);
    DEBUG_ASSERT(backLength <= length());
    if (start() <= end()) {
        auto endRange = between(end() - backLength, end());
        DEBUG_ASSERT(endRange.length() == backLength);
        second -= backLength;
        DEBUG_ASSERT(end() == endRange.start()); // adjacent
        return endRange;
    } else {
        auto endRange = between(end() + backLength, end());
        DEBUG_ASSERT(endRange.length() == backLength);
        second += backLength;
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

std::optional<IndexRange> intersect2(IndexRange lhs, IndexRange rhs) {
    if (lhs.start() < lhs.end()) {
        if (rhs.start() <= rhs.end()) {
            const SINT start = std::max(lhs.start(), rhs.start());
            const SINT end = std::min(lhs.end(), rhs.end());
            if (start <= end) {
                return IndexRange::between(start, end);
            }
        } else {
            DEBUG_ASSERT(!"Cannot intersect index ranges with different orientations");
            return std::nullopt;
        }
    } else if (lhs.start() > lhs.end()) {
        if (rhs.start() >= rhs.end()) {
            const SINT start = std::min(lhs.start(), rhs.start());
            const SINT end = std::max(lhs.end(), rhs.end());
            if (start >= end) {
                return IndexRange::between(start, end);
            }
        } else {
            DEBUG_ASSERT(!"Cannot intersect index ranges with different orientations");
            return std::nullopt;
        }
    } else {
        // Single point = empty range
        DEBUG_ASSERT(lhs.empty());
        DEBUG_ASSERT(lhs.start() == lhs.end());
        // Check if this point is located within the other range
        // and then return it
        if (rhs.start() <= rhs.end()) {
            if (lhs.start() >= rhs.start() && lhs.end() <= rhs.end()) {
                return lhs;
            }
        } else {
            if (lhs.start() <= rhs.start() && lhs.end() >= rhs.end()) {
                return lhs;
            }
        }
    }
    // disconnected
    return std::nullopt;
}

IndexRange span(IndexRange lhs, IndexRange rhs) {
    if (lhs.start() < lhs.end()) {
        if (rhs.start() <= rhs.end()) {
            const SINT start = std::min(lhs.start(), rhs.start());
            const SINT end = std::max(lhs.end(), rhs.end());
            DEBUG_ASSERT(start <= end);
            return IndexRange::between(start, end);
        } else {
            DEBUG_ASSERT(!"Cannot span index ranges with different orientations");
            return IndexRange();
        }
    } else if (lhs.start() > lhs.end()) {
        if (rhs.start() >= rhs.end()) {
            const SINT start = std::max(lhs.start(), rhs.start());
            const SINT end = std::min(lhs.end(), rhs.end());
            DEBUG_ASSERT(start >= end);
            return IndexRange::between(start, end);
        } else {
            DEBUG_ASSERT(!"Cannot span index ranges with different orientations");
            return IndexRange();
        }
    } else {
        // Single point = empty range
        DEBUG_ASSERT(lhs.empty());
        DEBUG_ASSERT(lhs.start() == lhs.end());
        if (rhs.start() <= rhs.end()) {
            const SINT start = std::min(lhs.start(), rhs.start());
            const SINT end = std::max(lhs.end(), rhs.end());
            DEBUG_ASSERT(start <= end);
            return IndexRange::between(start, end);
        } else {
            const SINT start = std::max(lhs.start(), rhs.start());
            const SINT end = std::min(lhs.end(), rhs.end());
            DEBUG_ASSERT(start >= end);
            return IndexRange::between(start, end);
        }
    }
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
