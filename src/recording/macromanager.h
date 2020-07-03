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
    MacroManager(EngineMaster* pMaster);

    void startRecording();
    void stopRecording();

  signals:
    void startMacroRecording(Macro* pMacro);
    void stopMacroRecording();

  private:
    ControlPushButton m_COToggleRecording;
    ControlObject m_CORecStatus;
    ControlObject m_CODeck;

    Macro* m_pRecordedMacro;
};
