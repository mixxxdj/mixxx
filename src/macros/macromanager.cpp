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
            &MacroManager::saveMacro);
}

void MacroManager::saveMacro(ChannelHandle channel, Macro macro) {
    qCDebug(macroLoggingCategory) << "Saving Macro for channel" << channel.handle();
    // TODO(xerus) add test, use proto serialization, obtain track id
    macro.dump();
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
            "INSERT INTO macros "
            "(track_id, state, label, content) "
            "VALUES "
            "(:trackId, :state, :label, :content)"));
    query.bindValue(":trackId", 1);
    query.bindValue(":state", 0);
    query.bindValue(":label", QString("testch%1").arg(QString::number(channel.handle())));
    query.bindValue(":content", macro.serialize());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }
    qCDebug(macroLoggingCategory) << "Macro saved";
}

MacroRecorder* MacroManager::getRecorder() {
    return m_pMacroRecorder.get();
}
