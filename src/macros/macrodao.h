#pragma once

#include <QtSql/QSqlDatabase>

#include "library/dao/dao.h"
#include "macros/macro.h"
#include "track/trackid.h"

class MacroDAO : public virtual DAO {
  public:
    void initialize(const QSqlDatabase& database) override;

    void saveMacro(TrackId trackId,
            QString label,
            QVector<MacroAction> actions,
            Macro::State state);
    QList<Macro> loadMacros(TrackId trackId) const;

  private:
    QSqlDatabase m_database;
};
