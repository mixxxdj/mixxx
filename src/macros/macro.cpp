#include "macro.h"

#include "util/assert.h"

void Macro::appendJump(double origin, double target) {
    VERIFY_OR_DEBUG_ASSERT(m_length < kMaxMacroSize) {
        return;
    }
    qCDebug(macros) << "Appending jump from position" << origin << "to" << target;
    actions[m_length].position = origin;
    actions[m_length].target = target;
    m_length++;
}

int Macro::getLength() {
    return m_length;
}

void Macro::clear() {
    qCDebug(macros) << "Clearing Macro";
    m_length = 0;
}

void Macro::dump() {
    for (size_t i = 0; i < m_length; ++i) {
        auto action = actions[i];
        qCDebug(macros) << "Jump from " << action.position << " to " << action.target;
    }
}
