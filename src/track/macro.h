#pragma once

#include <gtest/gtest_prod.h>

#include <QtCore>

#include "engine/engine.h"
#include "proto/macro.pb.h"
#include "track/macroaction.h"
#include "util/db/dbid.h"
namespace proto = mixxx::track::io;

constexpr int kMacrosPerChannel = 16;
extern const QLoggingCategory kMacroLoggingCategory;

/// A Macro stores a list of MacroActions as well as its current state and label.
class Macro {
  public:
    enum class StateFlag {
        Enabled = 1u,
        Looped = 2u,
    };
    Q_DECLARE_FLAGS(State, StateFlag);

    Macro();
    Macro(const QList<MacroAction>& actions,
            const QString& label,
            State state);

    bool isDirty() const;
    DbId getId() const;

    QString getLabel() const;
    void setLabel(const QString&);

    bool isEnabled() const;
    bool isLooped() const;
    const State& getState() const;
    void setState(StateFlag flag, bool enable = true);

    unsigned int size() const;

    const QList<MacroAction>& getActions() const;
    void addAction(const MacroAction& action);

    mixxx::audio::FramePos getStart() const;
    /// Sets the end of the Macro (relevant for looping).
    void setEnd(mixxx::audio::FramePos framePos);

    void clear();

  private:
    static QByteArray serialize(const QList<MacroAction>& actions);
    static QList<MacroAction> deserialize(const QByteArray& serialized);

    void setDirty(bool dirty = true);
    void setId(DbId id);

    bool m_bDirty = false;
    DbId m_iId;

    /// The list of actions. The first action marks the jump from end to start.
    QList<MacroAction> m_actions;
    QString m_label;
    State m_state;

    friend class MacroDAO;
    FRIEND_TEST(MacroTest, SerializeMacroActions);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Macro::State)

typedef std::shared_ptr<Macro> MacroPointer;

bool operator==(const Macro& m1, const Macro& m2);
inline bool operator!=(const Macro& m1, const Macro& m2) {
    return !(m1 == m2);
}

QDebug operator<<(QDebug debug, const Macro& macro);
