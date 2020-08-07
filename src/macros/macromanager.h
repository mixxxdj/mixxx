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
    MacroManager(mixxx::DbConnectionPoolPtr pDbConnectionPool);

    MacroRecorder* getRecorder();
    void setPlayerManager(PlayerManager*);

  public slots:
    void slotSaveMacroFromChannel(QVector<MacroAction>, ChannelHandle);
    void slotSaveMacro(QVector<MacroAction>, TrackId);

  private:
    std::unique_ptr<MacroRecorder> m_pMacroRecorder;
    std::unique_ptr<MacroDAO> m_pMacroDao;

    PlayerManager* m_pPlayerManager;
};
