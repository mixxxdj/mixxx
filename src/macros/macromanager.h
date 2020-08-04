#pragma once

#include <QObject>

#include "macrorecorder.h"
#include "track/trackid.h"
#include "util/db/dbconnectionpool.h"

class MacroManager : public QObject {
    Q_OBJECT
  public:
    MacroManager(mixxx::DbConnectionPoolPtr pDbConnectionPool);

    MacroRecorder* getRecorder();

    void saveMacro(TrackId trackId, QString label, QVector<MacroAction> actions);
    QList<Macro> loadMacros(TrackId trackId);

  public slots:
    void slotSaveMacro(ChannelHandle channel, QVector<MacroAction> actions);

  private:
    std::unique_ptr<MacroRecorder> m_pMacroRecorder;
    QSqlDatabase m_database;
};
