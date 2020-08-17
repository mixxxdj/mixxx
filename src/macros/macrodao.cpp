#include "macros/macrodao.h"

#include <QDebug>
#include <QSqlQuery>

#include "library/queryutil.h"

void MacroDAO::initialize(const QSqlDatabase& database) {
    m_database = database;
}

void MacroDAO::saveMacro(TrackId trackId, const Macro& macro, int number) const {
    return saveMacro(trackId, macro.getActions(), macro.getLabel(), macro.getState(), number);
}

void MacroDAO::saveMacro(TrackId trackId,
        QList<MacroAction> actions,
        QString label,
        Macro::State state,
        int number) const {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
            "INSERT INTO macros "
            "(track_id, number, label, state, content) "
            "VALUES "
            "(:trackId, :number, :label, :state, :content)"));
    if (number == 0) {
        number = getFreeNumber(trackId);
    }
    query.bindValue(":number", number);
    query.bindValue(":trackId", trackId.toVariant());
    query.bindValue(":label", label);
    query.bindValue(":state", (int)state);
    query.bindValue(":content", Macro::serialize(actions));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }
    qCDebug(macroLoggingCategory) << "Macro saved";
}

void MacroDAO::saveMacros(TrackId trackId, QMap<int, Macro> macros) const {
    for (const Macro& macro : macros) {
        saveMacro(trackId, macro);
    }
}

QSqlQuery MacroDAO::querySelect(QString columns, TrackId trackId) const {
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT %1 FROM macros WHERE track_id=:trackId")
                          .arg(columns));
    query.bindValue(":trackId", trackId.toVariant());
    return query;
}

int MacroDAO::getFreeNumber(TrackId trackId) const {
    QSqlQuery query = querySelect("number", trackId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return 1;
    }
    QList<int> numbers;
    while (query.next()) {
        numbers.append(query.value(0).toInt());
    }
    return Macro::getFreeSlot(numbers);
}

QMap<int, Macro> MacroDAO::loadMacros(TrackId trackId) const {
    QSqlQuery query = querySelect("*", trackId);
    QMap<int, Macro> result;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return result;
    }
    const QSqlRecord record = query.record();
    int numberColumn = record.indexOf("number");
    int stateColumn = record.indexOf("state");
    int labelColumn = record.indexOf("label");
    int contentColumn = record.indexOf("content");
    while (query.next()) {
        result.insert(
                query.value(numberColumn).toInt(),
                Macro(
                        Macro::deserialize(query.value(contentColumn).toByteArray()),
                        query.value(labelColumn).toString(),
                        Macro::State(query.value(stateColumn).toInt())));
    }
    return result;
}
