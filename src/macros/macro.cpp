#include "macro.h"

#include <QDebug>

Macro::Macro(bool enabled, bool loop, QString label, QVector<MacroAction> actions)
        : m_enabled(enabled),
          m_loop(loop),
          m_label(label),
          m_actions(actions) {
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

proto::Macro_Action* MacroAction::serialize() const {
    auto serialized = new proto::Macro_Action();
    serialized->set_position(position);
    serialized->set_target(target);
    serialized->set_type(static_cast<uint>(Type::Jump));
    return serialized;
}
