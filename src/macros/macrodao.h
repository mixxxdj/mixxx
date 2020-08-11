#pragma once

#include <QtSql/QSqlDatabase>

#include "library/dao/dao.h"
#include "macros/macro.h"
#include "track/trackid.h"

class MacroDAO : public virtual DAO {
  public:
    void initialize(const QSqlDatabase& database) override;

    void saveMacro(TrackId trackId,
            const QVector<MacroAction>& actions,
            const QString& label,
            Macro::State state);
    void saveMacro(TrackId trackId, const Macro& macro);
    QList<Macro> loadMacros(TrackId trackId) const;

  private:
    QSqlDatabase m_database;
};
