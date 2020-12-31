#include "track/macro.h"

#include <QDebug>

#include "util/assert.h"

proto::Macro_Action* MacroAction::serialize() const {
    auto serialized = new proto::Macro_Action();
    serialized->set_sourceframe(static_cast<uint64_t>(sourceFrame));
    serialized->set_targetframe(static_cast<uint64_t>(targetFrame));
    serialized->set_type(static_cast<uint32_t>(type));
    return serialized;
}

Macro::Macro(const QList<MacroAction>& actions, const QString& label, State state, int dbId)
        : m_bDirty(false),
          m_iId(dbId),
          m_actions(actions),
          m_label(label),
          m_state(state) {
}

// static
QList<MacroAction> Macro::deserialize(const QByteArray& serialized) {
    proto::Macro macroProto = proto::Macro();
    macroProto.ParseFromArray(serialized.data(), serialized.length());
    QList<MacroAction> result;
    result.reserve(macroProto.actions_size());
    for (const proto::Macro_Action& action : macroProto.actions()) {
        result.append(MacroAction(action.sourceframe(), action.targetframe()));
    }
    return result;
}

// static
QByteArray Macro::serialize(const QList<MacroAction>& actions) {
    proto::Macro macroProto;
    auto actionsProto = macroProto.mutable_actions();
    for (const MacroAction& action : actions) {
        actionsProto->AddAllocated(action.serialize());
    }
    auto string = macroProto.SerializeAsString();
    return QByteArray(string.data(), string.length());
}

bool Macro::isDirty() const {
    return m_bDirty;
}

int Macro::getId() const {
    return m_iId;
}

QString Macro::getLabel() const {
    return m_label;
}

void Macro::setLabel(const QString& label) {
    m_bDirty = true;
    m_label = label;
}

bool Macro::isEnabled() const {
    return m_state.testFlag(StateFlag::Enabled);
}

bool Macro::isLooped() const {
    return m_state.testFlag(StateFlag::Looped);
}

const Macro::State& Macro::getState() const {
    return m_state;
}

void Macro::setState(StateFlag flag, bool enable) {
    if (m_state.testFlag(flag) != enable) {
        m_bDirty = true;
        m_state.setFlag(flag, enable);
    }
}

bool Macro::isEmpty() const {
    return m_actions.isEmpty();
}

unsigned int Macro::size() const {
    return m_actions.size();
}

const QList<MacroAction>& Macro::getActions() const {
    return m_actions;
}

void Macro::addAction(const MacroAction& action) {
    m_bDirty = true;
    m_actions.append(action);
}

double Macro::getStartSamplePos() const {
    VERIFY_OR_DEBUG_ASSERT(!isEmpty()) {
        return 0;
    }
    return m_actions.first().getTargetPositionSample();
}

void Macro::setEnd(double framePos) {
    VERIFY_OR_DEBUG_ASSERT(!isEmpty()) {
        return;
    }
    // can't use replace because MacroAction is immutable
    m_actions.insert(0, MacroAction(framePos, m_actions.first().targetFrame));
    m_actions.removeAt(1);
}

void Macro::clear() {
    m_bDirty = true;
    m_actions.clear();
}

void Macro::setDirty(bool dirty) {
    m_bDirty = dirty;
}

void Macro::setId(int id) {
    m_iId = id;
}

bool operator==(const Macro& m1, const Macro& m2) {
    return m1.getId() == m2.getId() &&
            m1.getState() == m2.getState() &&
            m1.getLabel() == m2.getLabel() &&
            m1.getActions() == m2.getActions();
}

QDebug operator<<(QDebug debug, const MacroAction& action) {
    debug << "Jump from" << action.sourceFrame << "to" << action.targetFrame;
    return debug;
}
QDebug operator<<(QDebug debug, const Macro& macro) {
    debug << "Macro '" << macro.getLabel();
    debug << macro.getActions();
    return debug;
}
