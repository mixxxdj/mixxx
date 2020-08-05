#pragma once

#include <QtCore>

#include "proto/macro.pb.h"
namespace proto = mixxx::track::io;

const QLoggingCategory macroLoggingCategory("macros");

/// A MacroAction is the building block of a Macro.
/// It contains a position as well as the action to be taken at that position.
///
/// Note that currently only jumps to a target position are available, but that
/// is subject to change.
struct MacroAction {
    MacroAction(){};
    MacroAction(double position, double target)
            : position(position), target(target){};
    // use FramePos once https://github.com/mixxxdj/mixxx/pull/2961 is merged
    double position;
    double target;

    bool operator==(const MacroAction& other) const {
        return position == other.position && target == other.target;
    }

    inline bool operator!=(const MacroAction& other) const {
        return !operator==(other);
    }

    proto::Macro_Action* serialize() const;

    enum class Type : uint8_t {
        Jump = 0
    };
};

/// A Macro stores a list of MacroActions as well as its current state and label.
class Macro {
  public:
    static QByteArray serialize(const QVector<MacroAction>& actions);
    static QVector<MacroAction> deserialize(const QByteArray& serialized);

    enum class StateFlag {
        Enabled = 1u,
        Looped = 2u,
    };
    Q_DECLARE_FLAGS(State, StateFlag);

    Macro(QVector<MacroAction> actions, QString label, State state);

    const QVector<MacroAction> m_actions;
    QString m_label;
    State m_state;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Macro::State)

QDebug operator<<(QDebug debug, MacroAction action);
QDebug operator<<(QDebug debug, Macro macro);
