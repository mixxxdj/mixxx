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

struct MacroAction {
    MacroAction(){};
    MacroAction(double position, double target)
            : position(position), target(target){};
    double position;
    double target;
};

struct Macro {
    size_t m_length = 0;
    MacroAction actions[kMaxMacroSize];

    Macro() {
        std::fill_n(actions, kMaxMacroSize, MacroAction());
    }

    void appendHotcueJump(double origin, double target) {
        VERIFY_OR_DEBUG_ASSERT(m_length < kMaxMacroSize) {
            return;
        }
        qCDebug(macros) << "Appending jump from" << origin << "to" << target;
        actions[m_length].position = origin;
        actions[m_length].target = target;
        m_length++;
    };

    void dump() {
        for (size_t i = 0; i < m_length; ++i) {
            auto action = actions[i];
            qCDebug(macros) << "Jump from " << action.position << " to " << action.target;
        }
    }

    void clear() {
        qCDebug(macros) << "Clearing Macro";
        m_length = 0;
    }
};

class MacroManager : public RecordingManagerBase {
    Q_OBJECT
  public:
    MacroManager();

    void startRecording() override;
    void stopRecording() override;
    bool isRecordingActive() override;

    void appendHotcueJump(int channel, double origin, double target);

  private:
    ControlPushButton m_COToggleRecording;
    ControlObject m_CORecStatus;

    int m_activeChannel;
    std::atomic<MacroState> m_macroRecordingState;

    Macro m_recordedMacro;
};
