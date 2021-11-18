#pragma once

#include "library/dao/dao.h"
#include "track/macro.h"
#include "track/trackid.h"

class MacroDAO : public virtual DAO {
  public:
    bool saveMacro(TrackId trackId, Macro* macro, int slot) const;
    void saveMacros(TrackId trackId, const QMap<int, MacroPointer>& macros) const;

    QMap<int, MacroPointer> loadMacros(TrackId trackId) const;

  private:
    QSqlQuery querySelect(const QString& columns, TrackId trackId) const;
};
