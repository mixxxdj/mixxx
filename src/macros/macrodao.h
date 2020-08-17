#pragma once

#include <QtSql/QSqlDatabase>

#include "library/dao/dao.h"
#include "macros/macro.h"
#include "track/trackid.h"

class MacroDAO : public virtual DAO {
  public:
    void initialize(const QSqlDatabase& database) override;

    void saveMacro(TrackId trackId,
            QVector<MacroAction> actions,
            QString label,
            Macro::State state = Macro::State(),
            int number = 0) const;
    void saveMacro(TrackId trackId, const Macro& macro, int number = 0) const;
    void saveMacros(TrackId trackId, QMap<int, Macro> macros) const;

    int getFreeNumber(TrackId trackId) const;
    QMap<int, Macro> loadMacros(TrackId trackId) const;

  private:
    QSqlQuery querySelect(QString columns, TrackId trackId) const;

    QSqlDatabase m_database;
};
