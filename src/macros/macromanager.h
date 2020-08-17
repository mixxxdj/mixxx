#pragma once

#include <QObject>

#include "macros/macrodao.h"
#include "macros/macrorecorder.h"
#include "mixer/playermanager.h"
#include "track/trackid.h"
#include "util/db/dbconnectionpool.h"

class MacroManager : public QObject {
    Q_OBJECT
  public:
    MacroManager();

    MacroRecorder* getRecorder();
    void setPlayerManager(PlayerManager*);

  public slots:
    void slotSaveMacroFromChannel(QVector<MacroAction>, ChannelHandle);
    void slotSaveMacro(QVector<MacroAction>, TrackPointer);

  private:
    std::unique_ptr<MacroRecorder> m_pMacroRecorder;

    PlayerManager* m_pPlayerManager;
};
