#pragma once

#include <QObject>

#include "macrorecorder.h"
#include "mixer/playermanager.h"
#include "track/trackid.h"
#include "util/db/dbconnectionpool.h"

class MacroManager : public QObject {
    Q_OBJECT
  public:
    MacroManager(mixxx::DbConnectionPoolPtr pDbConnectionPool, PlayerManager* pPlayerManager);

    MacroRecorder* getRecorder();

    void saveMacro(TrackId trackId, QString label, QVector<MacroAction> actions);
    QList<Macro> loadMacros(TrackId trackId);

  public slots:
    void slotSaveMacro(ChannelHandle channel, QVector<MacroAction> actions);

  private:
    std::unique_ptr<MacroRecorder> m_pMacroRecorder;
    PlayerManager* m_pPlayerManager;
    QSqlDatabase m_database;
};
