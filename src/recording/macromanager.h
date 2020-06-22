#pragma once

#include <QObject>

#include "engine/enginemaster.h"
#include "mixer/playermanager.h"
#include "preferences/usersettings.h"
#include "recording/recordingmanagerbase.h"

#define MAX_MACRO_SIZE 1000
#define NO_DECK nullptr

struct MacroAction {
    MacroAction(){};
    MacroAction(double position, double target)
            : position(position), target(target){};
    double position;
    double target;
};

struct Macro {
    int length = 0;
    MacroAction actions[MAX_MACRO_SIZE];
    QString deck = NO_DECK;

    Macro() {
        qDebug() << "Constructing Macro";
        std::fill_n(actions, MAX_MACRO_SIZE, MacroAction());
    }

    void appendHotcueJump(double origin, double target) {
        qDebug() << "Macro: Appending jump";
        actions[length].position = origin;
        actions[length].target = target;
        length++;
    };

    void dump() {
        for (int i = 0; i < length; ++i) {
            auto action = actions[i];
            qDebug() << "Jump from " << action.position << " to " << action.target;
        }
    }

    void clear() {
        qDebug() << "Clearing Macro";
        length = 0;
        deck = NO_DECK;
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

  private slots:
    void slotHotcueActivate(double v);

  private:
    ControlProxy* m_pCPHotcueActivate;
    UserSettingsPointer m_pConfig;

    ControlProxy* m_pCPRecStatus;
    ControlObject* m_pCORecStatus;
    ControlPushButton* m_pToggleRecording;

    Macro* m_pRecordedMacro = new Macro();
};
