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
    MacroManager(mixxx::DbConnectionPoolPtr pDbConnectionPool, PlayerManager* pPlayerManager);

    MacroRecorder* getRecorder();

  public slots:
    void slotSaveMacroFromChannel(QVector<MacroAction> actions, ChannelHandle channel);
    void slotSaveMacro(QVector<MacroAction> actions, TrackPointer pTrack);

  private:
    std::unique_ptr<MacroRecorder> m_pMacroRecorder;
    std::unique_ptr<MacroDAO> m_pMacroDao;

    PlayerManager* m_pPlayerManager;
};
