#ifndef SAVEDQUERIESDAO_H
#define SAVEDQUERIESDAO_H

#include <QList>
#include <QSqlDatabase>
#include <QString>

#include "library/dao/dao.h"
#include "library/libraryfeature.h"
#include "util/dbid.h"

#define SAVEDQUERYTABLE "savedQueries"

const QString SAVEDQUERYTABLE_QUERY = "query";
const QString SAVEDQUERYTABLE_TITLE = "title";
const QString SAVEDQUERYTABLE_SELECTEDITEMS = "selectedItems";
const QString SAVEDQUERYTABLE_SORTORDER = "sortOrder";
const QString SAVEDQUERYTABLE_VSCROLLBARPOS = "vScrollbarPos";
const QString SAVEDQUERYTABLE_SORTCOLUMN = "sortColumn";
const QString SAVEDQUERYTABLE_SORTASCENDINGORDER = "sortAscendingOrder";
const QString SAVEDQUERYTABLE_PINNED = "pinned";

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

class SavedQueriesDAO : public DAO
{
  public:
    SavedQueriesDAO(QSqlDatabase& database);
    
    void initialize();
    void setSavedQueries(LibraryFeature* pFeature, const QList<SavedSearchQuery>& queries);
    QList<SavedSearchQuery> getSavedQueries(LibraryFeature* pFeature);
    
  private:
    static QString serializeItems(const QSet<DbId>& items) const;
    static QSet<DbId> deserializeItems(const QString& text) const;
    
    QSqlDatabase& m_database;
};

#endif // SAVEDQUERIESDAO_H
