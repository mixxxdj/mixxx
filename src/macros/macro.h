#pragma once

#include <QDebug>
#include <QtCore>

namespace {
constexpr size_t kMaxMacroSize = 1000;
const QLoggingCategory macros("macros");
} // namespace

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

/// A Macro stores a list of actions.
/// The action list is pre-populated at construction so that it can be written to in real-time code.
class Macro {
  public:
    MacroAction actions[kMaxMacroSize];

    Macro() {
        std::fill_n(actions, kMaxMacroSize, MacroAction());
    }

    /// Append a jump action to this Macro by assigning the next available slot.
    /// Only called from RT.
    void appendJump(double origin, double target);

    /// The amount of saved Actions
    int getLength();

    /// Clears the contents of this Macro by setting its length to 0.
    void clear();

    /// For debugging - dump all saved actions to debug output.
    void dump();

  private:
    size_t m_length = 0;
};
