#include <QString>

#include "library/dao/savedqueriesdao.h"
#include "library/libraryfeature.h"
#include "library/queryutil.h"

SavedQueriesDAO::SavedQueriesDAO(QSqlDatabase& database)
        : m_database(database) {

}

void SavedQueriesDAO::initialize() {
}

void SavedQueriesDAO::setSavedQueries(LibraryFeature* pFeature,
                                      const QList<SavedSearchQuery>& queries) {
    if (pFeature == nullptr) {
        return;
    }
    
    // First of all delete previous saved queries
    QString queryStr = "DELETE FROM " SAVEDQUERYTABLE " WHERE libraryFeature = :featureName";
    
    qDebug() << pFeature->getSettingsName();
    
    QSqlQuery query(m_database);
    query.prepare(queryStr);
    query.bindValue(":featureName", pFeature->getSettingsName());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    
    
    query.prepare("INSERT INTO " SAVEDQUERYTABLE 
                  "(libraryFeature, query, title, selectedItems,"
                  "sortOrder, vScrollbarPos, sortColumn, sortAscendingOrder, pinned) "
                  "VALUES (:libraryFeature, :query, :title, :selectedItems, "
                  ":sortOrder, :vScrollbarPos, :sortColumn, :sortAscendingOrder, :pinned)");
    
    for (const SavedSearchQuery& sQuery : queries) {
        query.bindValue(":libraryFeature", pFeature->getSettingsName());
        query.bindValue(":query", sQuery.query);
        query.bindValue(":title", sQuery.title);        
        query.bindValue(":selectedItems", serializeItems(sQuery.selectedItems));
        query.bindValue(":sortOrder", sQuery.sortOrder);
        query.bindValue(":vScrollbarPos", sQuery.vScrollBarPos);
        query.bindValue(":sortColumn", sQuery.sortColumn);
        query.bindValue(":sortAscendingOrder", sQuery.sortAscendingOrder);
        query.bindValue(":pinned", sQuery.pinned);
        
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
        }
    }
}

QList<SavedSearchQuery> SavedQueriesDAO::getSavedQueries(LibraryFeature* pFeature) {
    
    if (pFeature == nullptr) {
        return QList<SavedSearchQuery>();
    }
    
    QSqlQuery query(m_database);
    QString queryStr = "SELECT query, title, selectedItems, sortOrder, "
                       "vScrollbarPos, sortColumn, sortAscendingOrder, pinned "
                       "FROM " SAVEDQUERYTABLE 
                       " WHERE libraryFeature = :featureName";
    query.prepare(queryStr);
    query.bindValue(":featureName", pFeature->getSettingsName());
    
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    
    QList<SavedSearchQuery> res;
    while (query.next()) {
        SavedSearchQuery q;
        q.query = query.value(0).toString();
        q.title = query.value(1).toString();
        q.selectedItems = deserializeItems(query.value(2).toString());
        q.sortOrder = query.value(3).toString();
        q.vScrollBarPos = query.value(4).toInt();
        q.sortColumn = query.value(5).toInt();
        q.sortAscendingOrder = query.value(6).toBool();
        q.pinned = query.value(7).toBool();
        
        res << q;
    }
    return res;
}

QString SavedQueriesDAO::serializeItems(const QSet<DbId>& items) {
    QStringList ret;
    
    for (const DbId& id : items) {
        ret << id.toString();
    }
    return ret.join(" ");
}

QSet<DbId> SavedQueriesDAO::deserializeItems(const QString& text) {
    QSet<DbId> ret;
    QStringList items = text.split(" ");
    for (const QString& item : items) {
        ret.insert(DbId(QVariant(item)));
    }
    return ret;
}
