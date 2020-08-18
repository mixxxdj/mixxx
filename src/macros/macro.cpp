#include "macros/macro.h"

#include <QDebug>

proto::Macro_Action* MacroAction::serialize() const {
    auto serialized = new proto::Macro_Action();
    serialized->set_position(position);
    serialized->set_target(target);
    serialized->set_type(static_cast<uint>(Type::Jump));
    return serialized;
}

Macro::Macro(QList<MacroAction> actions, QString label, State state, int id)
        : m_iId(id),
          m_actions(actions),
          m_label(label),
          m_state(state) {
}

// static
QList<MacroAction> Macro::deserialize(QByteArray serialized) {
    proto::Macro macroProto = proto::Macro();
    macroProto.ParseFromArray(serialized.data(), serialized.length());
    QList<MacroAction> result;
    result.reserve(macroProto.actions_size());
    for (const proto::Macro_Action& action : macroProto.actions()) {
        result.append(MacroAction(action.position(), action.target()));
    }
    return result;
}

// static
QByteArray Macro::serialize(QList<MacroAction> actions) {
    proto::Macro macroProto;
    auto actionsProto = macroProto.mutable_actions();
    for (const MacroAction& action : actions) {
        actionsProto->AddAllocated(action.serialize());
    }
    auto string = macroProto.SerializeAsString();
    return QByteArray(string.data(), string.length());
}

// static
int Macro::getFreeSlot(QList<int> taken) {
    int number = 1;
    while (taken.contains(number))
        number++;
    return number;
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

void Macro::setLabel(QString label) {
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
    m_bDirty = true;
    m_state.setFlag(flag, enable);
}

bool Macro::isEmpty() const {
    return m_actions.isEmpty();
}

int Macro::size() const {
    return m_actions.size();
}

const QList<MacroAction>& Macro::getActions() const {
    return m_actions;
}

void Macro::addAction(const MacroAction& action) {
    m_bDirty = true;
    m_actions.append(action);
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

QDebug operator<<(QDebug debug, const MacroAction& action) {
    debug << "Jump from" << action.position << "to" << action.target;
    return debug;
}
QDebug operator<<(QDebug debug, const Macro& macro) {
    debug << "Macro '" << macro.getLabel();
    debug << macro.getActions();
    return debug;
}
