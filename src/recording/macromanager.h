#pragma once

#include <QDebug>
#include <QObject>
#include <QtCore>

#include "engine/enginemaster.h"
#include "mixer/playermanager.h"
#include "preferences/usersettings.h"
#include "recording/recordingmanagerbase.h"

namespace {
constexpr size_t kMaxMacroSize = 1000;
const QString kMacroRecordingKey = QStringLiteral("[MacroRecording]");
const QLoggingCategory macros("macros");
} // namespace

enum MacroState : uint8_t {
    /// Nothing is going on
    Disabled,
    /// Intermediate state, awaiting recording
    Armed,
    /// Actively recording
    Recording,
    /// Intermediate state, awaiting stop confirmation
    Stopping
};

/// A MacroAction is the smallest piece of a Macro.
/// It contains a position as well as the action to be taken at that position
/// (currently only jumps to a target position are available).
struct MacroAction {
    MacroAction(){};
    MacroAction(double position, double target)
            : position(position), target(target){};
    double position;
    double target;
};

/// A Macro stores a list of actions.
/// The action list is pre-populated at construction so that it can be written to in real-time code.
struct Macro {
    size_t m_length = 0;
    MacroAction actions[kMaxMacroSize];

    Macro() {
        std::fill_n(actions, kMaxMacroSize, MacroAction());
    }

    /// Append a jump action to this Macro by assigning the next available slot.
    /// Only called from RT.
    void appendJump(double origin, double target) {
        VERIFY_OR_DEBUG_ASSERT(m_length < kMaxMacroSize) {
            return;
        }
        qCDebug(macros) << "Appending jump from" << origin << "to" << target;
        actions[m_length].position = origin;
        actions[m_length].target = target;
        m_length++;
    };

    /// For debugging - dump all saved actions to debug output.
    void dump() {
        for (size_t i = 0; i < m_length; ++i) {
            auto action = actions[i];
            qCDebug(macros) << "Jump from " << action.position << " to " << action.target;
        }
    }

    /// Clears the contents of this Macro by setting its length to 0.
    void clear() {
        qCDebug(macros) << "Clearing Macro";
        m_length = 0;
    }
};

/// The MacroManager handles the recording of Macros and the [MacroRecording] controls.
class MacroManager : public RecordingManagerBase {
    Q_OBJECT
  public:
    MacroManager();

    void startRecording() override;
    void stopRecording() override;
    bool isRecordingActive() override;

    /// Tries to append this cue jump to the currently recorded Macro.
    /// Returns true if the currently recorded Macro was changed - so only if
    /// recording is active and the channel handle matches.
    /// Only called from RT.
    bool notifyCueJump(int channel, double origin, double target);

  private:
    /// Tries to claim the active recording for the current channel.
    /// Returns true if recording is active and the recorded channel matches
    /// the given one, or none was set and it was claimed.
    /// Only called from RT.
    bool claimRecording(int channel);

    ControlPushButton m_COToggleRecording;
    ControlObject m_CORecStatus;

    int m_activeChannel;
    std::atomic<MacroState> m_macroRecordingState;

    Macro m_recordedMacro;
};
