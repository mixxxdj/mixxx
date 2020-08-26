#include "library/dao/macrodao.h"

#include <QDebug>
#include <QSqlQuery>

#include "library/queryutil.h"

void MacroDAO::initialize(const QSqlDatabase& database) {
    m_database = database;
}

bool MacroDAO::saveMacro(TrackId trackId, Macro* macro, int slot) const {
    QSqlQuery query(m_database);
    if (macro->getId() == -1) {
        query.prepare(QStringLiteral(
                "INSERT INTO macros "
                "(track_id, slot, label, state, content) "
                "VALUES "
                "(:track_id, :slot, :label, :state, :content)"));
        if (slot == 0) {
            slot = getFreeSlot(trackId);
        }
    } else {
        VERIFY_OR_DEBUG_ASSERT(slot != 0) {
            slot = getFreeSlot(trackId);
        }
        query.prepare(QStringLiteral(
                "UPDATE macros SET "
                "track_id=:track_id,"
                "slot=:slot,"
                "label=:label,"
                "state=:state,"
                "content=:content"
                " WHERE id=:id"));
        query.bindValue(":id", macro->getId());
    }
    query.bindValue(":track_id", trackId.toVariant());
    query.bindValue(":slot", slot);
    query.bindValue(":label", macro->getLabel());
    query.bindValue(":state", macro->getState().operator int());
    query.bindValue(":content", Macro::serialize(macro->getActions()));

    if (query.exec()) {
        if (macro->getId() == -1) {
            macro->setId(query.lastInsertId().toInt());
        }
        macro->setDirty(false);
        return true;
    } else {
        LOG_FAILED_QUERY(query);
        return false;
    }
}

void MacroDAO::saveMacros(TrackId trackId, QMap<int, MacroPtr> macros) const {
    for (auto e : macros.toStdMap()) {
        // New macros (without an id) must always be marked as dirty
        auto pMacro = e.second;
        DEBUG_ASSERT(pMacro->getId() >= 0 || pMacro->isDirty());
        if (pMacro->isDirty()) {
            saveMacro(trackId, pMacro.get(), e.first);
        }
        // After saving each macro must have a valid id
        DEBUG_ASSERT(pMacro->getId() >= 0);
    }
}

QSqlQuery MacroDAO::querySelect(QString columns, TrackId trackId) const {
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT %1 FROM macros WHERE track_id=:trackId")
                          .arg(columns));
    query.bindValue(":trackId", trackId.toVariant());
    return query;
}

int MacroDAO::getFreeSlot(TrackId trackId) const {
    QSqlQuery query = querySelect("slot", trackId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return 1;
    }
    QList<int> taken;
    while (query.next()) {
        taken.append(query.value(0).toInt());
    }
    return Macro::getFreeSlot(taken);
}

QMap<int, MacroPtr> MacroDAO::loadMacros(TrackId trackId) const {
    QSqlQuery query = querySelect("*", trackId);
    QMap<int, MacroPtr> result;
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
        int slot = query.value(slotColumn).toInt();
        result.insert(
                slot,
                std::make_shared<Macro>(
                        Macro::deserialize(query.value(contentColumn).toByteArray()),
                        query.value(labelColumn).toString(),
                        Macro::State(query.value(stateColumn).toInt()),
                        query.value(idColumn).toInt()));
    }
    for (int i = 1; i <= kMacrosPerTrack; ++i) {
        if (!result.contains(i)) {
            result.insert(i, std::make_shared<Macro>());
        }
    }
    return result;
}
