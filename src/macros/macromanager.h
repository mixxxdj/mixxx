#pragma once

#include <QObject>

#include "macrorecorder.h"
#include "util/db/dbconnectionpool.h"

class MacroManager : public QObject {
    Q_OBJECT
  public:
    MacroManager(mixxx::DbConnectionPoolPtr pDbConnectionPool);

    MacroRecorder* getRecorder();

  public slots:
    void saveMacro(ChannelHandle channel, QVector<MacroAction> actions);

  private:
    std::unique_ptr<MacroRecorder> m_pMacroRecorder;
    QSqlDatabase m_database;
};
