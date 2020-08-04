#include "macromanager.h"

#include <QDebug>
#include <QSqlQuery>

#include "library/queryutil.h"
#include "util/db/dbconnectionpooled.h"

MacroManager::MacroManager(mixxx::DbConnectionPoolPtr pDbConnectionPool)
        : m_pMacroRecorder(std::make_unique<MacroRecorder>()),
          m_database(mixxx::DbConnectionPooled(pDbConnectionPool)) {
    connect(getRecorder(),
            &MacroRecorder::saveMacro,
            this,
            &MacroManager::slotSaveMacro);
}

void MacroManager::slotSaveMacro(ChannelHandle channel, QVector<MacroAction> actions) {
    qCDebug(macroLoggingCategory) << "Saving Macro for channel" << channel.handle();
    saveMacro(TrackId(1), "Unnamed Macro", actions);
}

void MacroManager::saveMacro(TrackId trackId, QString label, QVector<MacroAction> actions) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
            "INSERT INTO macros "
            "(track_id, label, state, content) "
            "VALUES "
            "(:trackId, :label, :state, :content)"));
    // TODO(xerus) obtain trackId
    query.bindValue(":trackId", trackId.toVariant());
    query.bindValue(":label", label);
    query.bindValue(":state", 0u);
    query.bindValue(":content", Macro::serialize(actions));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }
    qCDebug(macroLoggingCategory) << "Macro saved";
}

QList<Macro> MacroManager::loadMacros(TrackId trackId) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
            "SELECT * FROM macros WHERE track_id=:trackId"));
    query.bindValue(":trackId", trackId.toVariant());
    QList<Macro> result;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return result;
    }
    const QSqlRecord record = query.record();
    int stateColumn = record.indexOf("state");
    int labelColumn = record.indexOf("label");
    int contentColumn = record.indexOf("content");
    while (query.next()) {
        uint state = query.value(stateColumn).toUInt();
        result.append(Macro(
                state & 1u,
                state & 2u,
                query.value(labelColumn).toString(),
                Macro::deserialize(query.value(contentColumn).toByteArray())));
    }
    return result;
}

MacroRecorder* MacroManager::getRecorder() {
    return m_pMacroRecorder.get();
}
