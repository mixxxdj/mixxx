#include "macros/macro.h"

#include <QDebug>
#include <utility>

Macro::Macro(QVector<MacroAction> actions, QString label, State state)
        : m_actions(std::move(actions)),
          m_label(std::move(label)),
          m_state(state) {
}

// static
QVector<MacroAction> Macro::deserialize(const QByteArray& serialized) {
    proto::Macro macroProto = proto::Macro();
    macroProto.ParseFromArray(serialized.data(), serialized.length());
    QVector<MacroAction> result(macroProto.actions_size());
    int i = 0;
    for (const proto::Macro_Action& action : macroProto.actions()) {
        result.replace(i++, MacroAction(action.position(), action.target()));
    }
    return result;
}

// static
QByteArray Macro::serialize(const QVector<MacroAction>& actions) {
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

proto::Macro_Action* MacroAction::serialize() const {
    auto serialized = new proto::Macro_Action();
    serialized->set_position(position);
    serialized->set_target(target);
    serialized->set_type(static_cast<uint>(Type::Jump));
    return serialized;
}

QDebug operator<<(QDebug debug, MacroAction action) {
    debug << "Jump from" << action.position << "to" << action.target;
    return debug;
}
QDebug operator<<(QDebug debug, Macro macro) {
    debug << "Macro '" << macro.m_label << "' (" << macro.m_state << ")";
    debug << macro.m_actions;
    return debug;
}
