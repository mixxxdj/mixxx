#pragma once

#include <QtCore>

#include "proto/macro.pb.h"
namespace proto = mixxx::track::io;

const QLoggingCategory macroLoggingCategory("macros");
constexpr int kMacrosPerTrack = 16;

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
    static QByteArray serialize(QList<MacroAction> actions);
    static QList<MacroAction> deserialize(QByteArray serialized);
    static int getFreeSlot(QList<int> taken);

    enum class StateFlag {
        Enabled = 1u,
        Looped = 2u,
    };
    Q_DECLARE_FLAGS(State, StateFlag);

    explicit Macro(QList<MacroAction> actions = {},
            QString label = "",
            State state = State(StateFlag::Enabled),
            int dbId = -1);

    bool isDirty() const;
    int getId() const;

    QString getLabel() const;
    void setLabel(QString);

    bool isEnabled() const;
    bool isLooped() const;
    const State& getState() const;
    void setState(StateFlag flag, bool enable = true);

    bool isEmpty() const;
    int size() const;

    const QList<MacroAction>& getActions() const;
    void addAction(const MacroAction& action);

    void clear();

  private:
    void setDirty(bool dirty);
    void setId(int id);

    bool m_bDirty;
    int m_iId;

    QList<MacroAction> m_actions;
    QString m_label;
    State m_state;

    friend class MacroDAO;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Macro::State)

typedef std::shared_ptr<Macro> MacroPtr;

bool operator==(const Macro& m1, const Macro& m2);
QDebug operator<<(QDebug debug, const MacroAction& action);
QDebug operator<<(QDebug debug, const Macro& macro);
