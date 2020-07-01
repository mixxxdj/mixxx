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
const QLoggingCategory macros("macros");
}

struct MacroAction {
    MacroAction(){};
    MacroAction(double position, double target)
            : position(position), target(target){};
    double position;
    double target;
};

struct Macro {
    int m_length = 0;
    MacroAction actions[kMaxMacroSize];

    Macro() {
        std::fill_n(actions, kMaxMacroSize, MacroAction());
    }

    void appendHotcueJump(double origin, double target) {
        qCDebug(macros) << "Appending jump from" << origin << "to" << target;
        actions[m_length].position = origin;
        actions[m_length].target = target;
        m_length++;
    };

    void dump() {
        for (int i = 0; i < m_length; ++i) {
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
    MacroManager(
            UserSettingsPointer pConfig,
            EngineMaster* pMaster,
            PlayerManager* pPlayerManager);
    ~MacroManager() override;

    void startRecording();
    void stopRecording();

  signals:
    void startMacroRecording(Macro* pMacro);
    void stopMacroRecording();

  private:
    UserSettingsPointer m_pConfig;

    ControlObject* m_pCORecStatus;
    ControlPushButton* m_pToggleRecording;

    Macro* m_pRecordedMacro;
};
