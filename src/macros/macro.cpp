#include "macro.h"

#include <QDebug>

#include "util/assert.h"

Macro::Macro(QByteArray serialized) {
    proto::Macro macroProto = proto::Macro();
    macroProto.ParseFromArray(serialized.data(), serialized.length());
    for (auto action : macroProto.actions()) {
        appendJump(action.origin(), action.target());
    }
}

void Macro::appendJump(double sourceFramePos, double destFramePos) {
    VERIFY_OR_DEBUG_ASSERT(m_length < kMaxSize) {
        return;
    }
    qCDebug(macroLoggingCategory)
            << "Length" << m_length
            << ": Appending jump from position" << sourceFramePos << "to" << destFramePos;
    actions[m_length].position = sourceFramePos;
    actions[m_length].target = destFramePos;
    m_length++;
}

void Macro::clear() {
    qCDebug(macroLoggingCategory) << "Clearing Macro";
    m_length = 0;
}

int Macro::getLength() const {
    return m_length;
}

void Macro::dump() const {
    for (size_t i = 0; i < m_length; ++i) {
        auto action = actions[i];
        qCDebug(macroLoggingCategory) << "Jump from " << action.position << " to " << action.target;
    }
}

QByteArray Macro::serialize() const {
    proto::Macro macroProto;
    auto actionsProto = macroProto.mutable_actions();
    for (size_t i = 0; i < m_length; ++i) {
        actionsProto->AddAllocated(actions[i].serialize());
    }
    auto string = macroProto.SerializeAsString();
    return QByteArray(string.data(), string.length());
}
