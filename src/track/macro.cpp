#include "track/macro.h"

#include <QDebug>

#include "util/assert.h"

const QLoggingCategory kMacroLoggingCategory("macros");

// static
QList<MacroAction> Macro::deserialize(const QByteArray& serialized) {
    proto::Macro macroProto = proto::Macro();
    QList<MacroAction> result;
    VERIFY_OR_DEBUG_ASSERT(macroProto.ParseFromArray(serialized.data(), serialized.length())) {
        qCDebug(kMacroLoggingCategory) << "Failed parsing Macro from " << serialized;
        return result;
    }
    result.reserve(macroProto.actions_size());
    for (const proto::Macro_Action& action : macroProto.actions()) {
        result.append(MacroAction(mixxx::audio::FramePos(action.source_frame()),
                mixxx::audio::FramePos(action.target_frame())));
    }
    return result;
}

// static
QByteArray Macro::serialize(const QList<MacroAction>& actions) {
    proto::Macro macroProto;
    auto* actionsProto = macroProto.mutable_actions();
    for (const MacroAction& action : actions) {
        actionsProto->Add(action.serialize());
    }
    return QByteArray::fromStdString(macroProto.SerializeAsString());
}

Macro::Macro()
        : m_state(StateFlag::Enabled) {
}

Macro::Macro(const QList<MacroAction>& actions, const QString& label, State state)
        : m_actions(actions),
          m_label(label),
          m_state(state) {
}

bool Macro::isDirty() const {
    return m_bDirty;
}

DbId Macro::getId() const {
    return m_iId;
}

QString Macro::getLabel() const {
    return m_label;
}

void Macro::setLabel(const QString& label) {
    setDirty();
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
        setDirty();
        m_state.setFlag(flag, enable);
    }
}

bool Macro::isEmpty() const {
    return getActions().isEmpty();
}

unsigned int Macro::size() const {
    return m_actions.size();
}

const QList<MacroAction>& Macro::getActions() const {
    return m_actions;
}

void Macro::addAction(const MacroAction& action) {
    setDirty();
    m_actions.append(action);
}

mixxx::audio::FramePos Macro::getStart() const {
    VERIFY_OR_DEBUG_ASSERT(!getActions().isEmpty()) {
        return mixxx::audio::FramePos(0);
    }
    return m_actions.first().getTargetPosition();
}

void Macro::setEnd(mixxx::audio::FramePos framePos) {
    VERIFY_OR_DEBUG_ASSERT(!getActions().isEmpty()) {
        return;
    }
    // can't use replace because MacroAction is immutable
    m_actions.insert(0, MacroAction(framePos, m_actions.first().getTargetPosition()));
    m_actions.removeAt(1);
}

void Macro::clear() {
    setDirty();
    m_actions.clear();
}

void Macro::setDirty(bool dirty) {
    m_bDirty = dirty;
}

void Macro::setId(DbId id) {
    m_iId = id;
}

bool operator==(const Macro& m1, const Macro& m2) {
    return m1.getId() == m2.getId() &&
            m1.getState() == m2.getState() &&
            m1.getLabel() == m2.getLabel() &&
            m1.getActions() == m2.getActions();
}

QDebug operator<<(QDebug debug, const Macro& macro) {
    debug << "Macro '" << macro.getLabel();
    debug << macro.getActions();
    return debug;
}
