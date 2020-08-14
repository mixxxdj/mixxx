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
            Macro::State state = Macro::State(),
            int number = 0);
    void saveMacro(TrackId trackId, const Macro& macro, int number = 0);

    int getFreeNumber(TrackId trackId) const;
    QMap<int, Macro> loadMacros(TrackId trackId) const;

  private:
    QSqlQuery querySelect(QString columns, TrackId trackId) const;

    QSqlDatabase m_database;
};
