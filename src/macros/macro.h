#pragma once

#include <QDebug>
#include <QtCore>

const QLoggingCategory macroLoggingCategory("macros");

/// A MacroAction is the smallest piece of a Macro.
/// It contains a position as well as the action to be taken at that position
/// (currently only jumps to a target position are available).
struct MacroAction {
    MacroAction(){};
    MacroAction(double position, double target)
            : position(position), target(target){};
    /// TODO(xerus) use FramePos once https://github.com/mixxxdj/mixxx/pull/2861 is merged
    double position;
    double target;
};

/// A Macro stores a list of MacroActions.
/// The maximum size is fixed at a generous kMaxSize to prevent resizing in RT.
class Macro {
  public:
    static const int kMaxSize = 1000;

    MacroAction actions[kMaxSize];

    /// Append a jump action to this Macro by assigning the next available slot.
    /// Only called from RT.
    void appendJump(double origin, double target);

    /// Clears the contents of this Macro by setting its length to 0.
    void clear();

    /// Number of saved Actions.
    int getLength() const;

    QByteArray serialize() const;

    /// For debugging - dump all saved actions to debug output.
    void dump() const;

  private:
    size_t m_length = 0;
};
