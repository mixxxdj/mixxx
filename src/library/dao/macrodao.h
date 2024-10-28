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
    enum class MacroColumn {
        Id,
        TrackId,
        Slot,
        Label,
        State,
        Content
    };
    QString columnToString(MacroColumn column) const;
    QSqlQuery querySelect(const std::vector<MacroColumn>& columns, TrackId trackId) const;
};
