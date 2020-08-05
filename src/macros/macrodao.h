#pragma once

#include <QtSql/QSqlDatabase>

#include "macros/macro.h"
#include "track/trackid.h"

class MacroDAO {
  public:
    MacroDAO(const QSqlDatabase& database);

    void saveMacro(TrackId trackId,
            QString label,
            QVector<MacroAction> actions,
            Macro::State state);
    QList<Macro> loadMacros(TrackId trackId);

  private:
    QSqlDatabase m_database;
};
