#ifndef SAVEDQUERIESDAO_H
#define SAVEDQUERIESDAO_H

#include <QList>
#include <QSqlDatabase>
#include <QString>

#include "library/dao/dao.h"
#include "util/dbid.h"

#define SAVEDQUERYTABLE "savedQueries"

// This struct allows to save some data to allow interaction between
// the search bar and the library features
struct SavedSearchQuery {
    
    SavedSearchQuery() : 
        vScrollBarPos(-1),
        sortColumn(-1), 
        sortAscendingOrder(false), 
        pinned(false),
        id(-1) {}
    
    SavedSearchQuery(const SavedSearchQuery& other) = default;
    SavedSearchQuery& operator=(const SavedSearchQuery& other) = default;
    
    QString query;
    QString title;
    QSet<DbId> selectedItems;
    QString sortOrder;
    
    int vScrollBarPos;
    int sortColumn;
    bool sortAscendingOrder;
    bool pinned;
    int id;
};

class LibraryFeature;

class SavedQueriesDAO : public DAO
{
  public:
    SavedQueriesDAO(QSqlDatabase& database);
    
    void initialize();
    SavedSearchQuery saveQuery(LibraryFeature* pFeature, SavedSearchQuery sQuery);
    QList<SavedSearchQuery> getSavedQueries(const QString& settingsName) const;
    QList<SavedSearchQuery> getSavedQueries(const LibraryFeature *pFeature) const;
    SavedSearchQuery getSavedQuery(int id) const;
    SavedSearchQuery moveToFirst(LibraryFeature* pFeature, const SavedSearchQuery& sQuery);
    SavedSearchQuery moveToFirst(LibraryFeature* pFeature, int id);
    
  private:
    static QString serializeItems(const QSet<DbId>& items);
    static QSet<DbId> deserializeItems(const QString& text);
    static SavedSearchQuery valueToQuery(const QSqlQuery& query);
    
    static const QString kSelectStart;
    
    QSqlDatabase& m_database;
};

#endif // SAVEDQUERIESDAO_H
