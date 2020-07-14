#include "macro.h"

#include <QDebug>

#include "proto/macro.pb.h"
#include "util/assert.h"

using namespace mixxx::track;

Macro::Macro(QByteArray serialized) {
    io::Macro macroProto = io::Macro();
    macroProto.ParseFromArray(serialized.data(), serialized.length());
    for (auto action : macroProto.actions()) {
        appendJump(action.origin(), action.target());
    }
}

void Macro::appendJump(double origin, double target) {
    VERIFY_OR_DEBUG_ASSERT(m_length < kMaxSize) {
        return;
    }
    qCDebug(macroLoggingCategory)
            << "Length" << m_length
            << ": Appending jump from position" << origin << "to" << target;
    actions[m_length].position = origin;
    actions[m_length].target = target;
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
    io::Macro macroProto;
    for (size_t i = 0; i < m_length; ++i) {
        MacroAction action = actions[i];
        io::Macro_Action* newAction = macroProto.add_actions();
        // TODO(xerus) add type enum
        newAction->set_type(1);
        newAction->set_origin(action.position);
        newAction->set_target(action.target);
    }
    auto string = macroProto.SerializeAsString();
    return QByteArray(string.data(), string.length());
}
