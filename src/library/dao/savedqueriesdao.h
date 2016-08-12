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
    QString query;
    QString title;
    QSet<DbId> selectedItems;
    QString sortOrder;
    
    int vScrollBarPos;
    int sortColumn;
    bool sortAscendingOrder;
    bool pinned;
};

class LibraryFeature;

class SavedQueriesDAO : public DAO
{
  public:
    SavedQueriesDAO(QSqlDatabase& database);
    
    void initialize();
    void setSavedQueries(LibraryFeature* pFeature, const QList<SavedSearchQuery>& queries);
    QList<SavedSearchQuery> getSavedQueries(LibraryFeature* pFeature);
    
  private:
    static QString serializeItems(const QSet<DbId>& items);
    static QSet<DbId> deserializeItems(const QString& text);
    
    QSqlDatabase& m_database;
};

#endif // SAVEDQUERIESDAO_H
