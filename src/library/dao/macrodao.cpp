#include "library/dao/macrodao.h"

#include <QSqlQuery>

#include "library/queryutil.h"

bool MacroDAO::saveMacro(TrackId trackId, Macro* macro, int slot) const {
    QSqlQuery query(m_database);
    DEBUG_ASSERT(slot > 0);
    if (!macro->getId().isValid()) {
        query.prepare(QStringLiteral(
                "INSERT INTO macros "
                "(track_id, slot, label, state, content) "
                "VALUES "
                "(:track_id, :slot, :label, :state, :content)"));
    } else {
        query.prepare(QStringLiteral(
                "UPDATE macros SET "
                "track_id=:track_id,"
                "slot=:slot,"
                "label=:label,"
                "state=:state,"
                "content=:content"
                " WHERE id=:id"));
        query.bindValue(":id", macro->getId().value());
    }
    query.bindValue(":track_id", trackId.toVariant());
    query.bindValue(":slot", slot);
    query.bindValue(":label", macro->getLabel());
    query.bindValue(":state", macro->getState().operator int());
    query.bindValue(":content", Macro::serialize(macro->getActions()));

    if (query.exec()) {
        if (!macro->getId().isValid()) {
            macro->setId(DbId(query.lastInsertId()));
        } else {
            DEBUG_ASSERT(query.lastInsertId() == macro->getId().value());
        }
        macro->setDirty(false);
        return true;
    } else {
        LOG_FAILED_QUERY(query);
        return false;
    }
}

void MacroDAO::saveMacros(TrackId trackId, const QMap<int, MacroPointer>& macros) const {
    for (auto it = macros.constBegin(); it != macros.constEnd(); ++it) {
        auto pMacro = it.value();
        // Don't save placeholder Macros
        if (pMacro->isEmpty() && !pMacro->isDirty()) {
            return;
        }
        // Newly recorded macros must be dirty
        DEBUG_ASSERT(pMacro->getId().isValid() || pMacro->isDirty());
        if (pMacro->isDirty()) {
            saveMacro(trackId, pMacro.get(), it.key());
        }
        // After saving each macro must have a valid id and not be dirty
        DEBUG_ASSERT(pMacro->getId().isValid() && !pMacro->isDirty());
    }
}

QString MacroDAO::columnToString(MacroColumn column) const {
    switch (column) {
    case MacroColumn::Id:
        return "id";
    case MacroColumn::TrackId:
        return "track_id";
    case MacroColumn::Slot:
        return "slot";
    case MacroColumn::Label:
        return "label";
    case MacroColumn::State:
        return "state";
    case MacroColumn::Content:
        return "content";
    default:
        DEBUG_ASSERT(false);
        return QString();
    }
}

QSqlQuery MacroDAO::querySelect(const std::vector<MacroColumn>& columns, TrackId trackId) const {
    QStringList columnNames;
    for (const auto& column : columns) {
        columnNames << columnToString(column);
    }
    QString columnList = columnNames.join(", ");

    QSqlQuery query(m_database);
    query.prepare(QString("SELECT %1 FROM macros WHERE track_id=:trackId").arg(columnList));
    query.bindValue(":trackId", trackId.toVariant());
    return query;
}

QMap<int, MacroPointer> MacroDAO::loadMacros(TrackId trackId) const {
    std::vector<MacroColumn> columns = {
            MacroColumn::Id,
            MacroColumn::Slot,
            MacroColumn::State,
            MacroColumn::Label,
            MacroColumn::Content};
    QSqlQuery query = querySelect(columns, trackId);
    QMap<int, MacroPointer> result;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return result;
    }
    const QSqlRecord record = query.record();
    int idColumn = record.indexOf("id");
    int slotColumn = record.indexOf("slot");
    int stateColumn = record.indexOf("state");
    int labelColumn = record.indexOf("label");
    int contentColumn = record.indexOf("content");
    while (query.next()) {
        auto macro = std::make_shared<Macro>(
                Macro::deserialize(query.value(contentColumn).toByteArray()),
                query.value(labelColumn).toString(),
                Macro::State(query.value(stateColumn).toInt()));
        macro->setId(DbId(query.value(idColumn)));
        result.insert(query.value(slotColumn).toInt(), macro);
    }
    return result;
}
