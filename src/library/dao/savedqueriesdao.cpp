#include <QString>

#include "library/dao/savedqueriesdao.h"
#include "library/libraryfeature.h"
#include "library/queryutil.h"

const QString SavedQueriesDAO::kSelectStart =
        "SELECT query, title, selectedItems, sortOrder, "
        "vScrollbarPos, sortColumn, sortAscendingOrder, pinned, id ";

SavedQueriesDAO::SavedQueriesDAO(QSqlDatabase& database)
        : m_database(database) {

}

void SavedQueriesDAO::initialize() {
}

SavedSearchQuery SavedQueriesDAO::saveQuery(LibraryFeature* pFeature,
                                            SavedSearchQuery sQuery) {
    if (pFeature == nullptr) {
        return SavedSearchQuery();
    }
    
    QSqlQuery query(m_database);    
    query.prepare("INSERT INTO " SAVEDQUERYTABLE 
                  "(libraryFeature, query, title, selectedItems,"
                  "sortOrder, vScrollbarPos, sortColumn, sortAscendingOrder, pinned) "
                  "VALUES (:libraryFeature, :query, :title, :selectedItems, "
                  ":sortOrder, :vScrollbarPos, :sortColumn, :sortAscendingOrder, :pinned)");
    
    query.bindValue(":libraryFeature", pFeature->getSettingsName());
    query.bindValue(":query", sQuery.query);
    query.bindValue(":title", sQuery.title);        
    query.bindValue(":selectedItems", serializeItems(sQuery.selectedItems));
    query.bindValue(":sortOrder", sQuery.sortOrder);
    query.bindValue(":vScrollbarPos", sQuery.vScrollBarPos);
    query.bindValue(":sortColumn", sQuery.sortColumn);
    query.bindValue(":sortAscendingOrder", (int) sQuery.sortAscendingOrder);
    query.bindValue(":pinned", (int) sQuery.pinned);
    
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    
    if (!query.exec("SELECT id FROM " SAVEDQUERYTABLE
                    " WHERE ROWID=last_inserted_rowid()")) {
        LOG_FAILED_QUERY(query);
    }
    
    query.next();
    sQuery.id = query.value(0).toInt();
    return sQuery;
}

QList<SavedSearchQuery> SavedQueriesDAO::getSavedQueries(const QString& settingsName) const {    
    QSqlQuery query(m_database);
    QString queryStr = kSelectStart + 
                       "FROM " SAVEDQUERYTABLE 
                       " WHERE libraryFeature = :featureName "
                       "ORDER BY pinned DESC, id DESC";
    query.prepare(queryStr);
    query.bindValue(":featureName", settingsName);
    
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    
    QList<SavedSearchQuery> res;
    while (query.next()) {
        res << valueToQuery(query);
    }
    return res;
}

QList<SavedSearchQuery> SavedQueriesDAO::getSavedQueries(const LibraryFeature* pFeature) const {
    if (pFeature == nullptr) {
        return QList<SavedSearchQuery>();
    }
    
    return getSavedQueries(pFeature->getSettingsName());
}

SavedSearchQuery SavedQueriesDAO::getSavedQuery(int id) const {
    QSqlQuery query(m_database);
    QString queryStr = kSelectStart +
                       "FROM " SAVEDQUERYTABLE 
                       " WHERE id = :id "
                       "ORDER BY id DESC";
    query.prepare(queryStr);
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    
    query.next();
    return valueToQuery(query);
}

SavedSearchQuery SavedQueriesDAO::moveToFirst(LibraryFeature* pFeature,
                                              const SavedSearchQuery& sQuery) {
    // To move to the first item we delete the item and insert againt it
    // to assign it a new ID
    deleteSavedQuery(sQuery.id);
    return saveQuery(pFeature, sQuery);
}

SavedSearchQuery SavedQueriesDAO::moveToFirst(LibraryFeature* pFeature, int id) {
    return moveToFirst(pFeature, getSavedQuery(id));
}

bool SavedQueriesDAO::updateSavedQuery(const SavedSearchQuery& sQuery) {    
    QSqlQuery query(m_database);
    query.prepare("UPDATE " SAVEDQUERYTABLE " SET "
            "query = :query, title = :title, selectedItems = :selectedItems, "
            "sortOrder = :sortOrder, vScrollbarPos = :vScrollbarPos, "
            "sortColumn = :sortColumn, "
            "sortAscendingOrder = :sortAscendingOrder, pinned = :pinned "
            "WHERE id = :id");
    
    query.bindValue(":query", sQuery.query);
    query.bindValue(":title", sQuery.title);        
    query.bindValue(":selectedItems", serializeItems(sQuery.selectedItems));
    query.bindValue(":sortOrder", sQuery.sortOrder);
    query.bindValue(":vScrollbarPos", sQuery.vScrollBarPos);
    query.bindValue(":sortColumn", sQuery.sortColumn);
    query.bindValue(":sortAscendingOrder", (int) sQuery.sortAscendingOrder);
    query.bindValue(":pinned", (int) sQuery.pinned);
    query.bindValue(":id", sQuery.id);
    
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    return true;
}

bool SavedQueriesDAO::deleteSavedQuery(int id) {
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM " SAVEDQUERYTABLE " WHERE id=:id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    return true;
}

bool SavedQueriesDAO::exists(const SavedSearchQuery& sQuery) {
    return getQueryId(sQuery) >= 0;
}

int SavedQueriesDAO::getQueryId(const SavedSearchQuery& sQuery) {
    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM " SAVEDQUERYTABLE " "
                  "WHERE query=:query AND title=:title");
    query.bindValue(":query", sQuery.query);
    query.bindValue(":title", sQuery.title);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    if (query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

QString SavedQueriesDAO::serializeItems(const QSet<DbId>& items) {
    QStringList ret;
    
    for (const DbId& id : items) {
        ret << id.toString();
    }
    return ret.join(" ");
}

QSet<DbId> SavedQueriesDAO::deserializeItems(QString text) {
    QSet<DbId> ret;
    QTextStream ss(&text);
    while (!ss.atEnd()) {
        int value;
        ss >> value;
        ret.insert(DbId(QVariant(value)));
    }    
    return ret;
}

SavedSearchQuery SavedQueriesDAO::valueToQuery(const QSqlQuery& query) {
    SavedSearchQuery q;
    q.query = query.value(0).toString();
    q.title = query.value(1).toString();
    q.selectedItems = deserializeItems(query.value(2).toString());
    q.sortOrder = query.value(3).toString();
    q.vScrollBarPos = query.value(4).toInt();
    q.sortColumn = query.value(5).toInt();
    q.sortAscendingOrder = query.value(6).toBool();
    q.pinned = query.value(7).toBool();
    q.id = query.value(8).toInt();
    
    return q;
}
