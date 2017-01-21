#ifndef SAVEDQUERIESDAO_H
#define SAVEDQUERIESDAO_H

#include <QList>
#include <QSqlDatabase>
#include <QString>

#include "library/dao/dao.h"
#include "util/dbid.h"

#define SAVEDQUERYTABLE "savedQueries"

enum SavedQueryColumns {
    ID,
    LIBRARYFEATURE,
    QUERY,
    TITLE,
    SELECTEDITEMS,
    SORTORDER,
    VSCROLLBARPOS,
    SORTCOLUMN,
    SORTASCENDINGORDER,
    PINNED,
    
    // NUM_COLUMNS should be always the last item
    NUM_COLUMNS
};

const QString SAVEDQUERYTABLE_ID = "id";
const QString SAVEDQUERYTABLE_LIBRARYFEATURE = "libraryFeature";
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
    
    SavedSearchQuery() : 
        vScrollBarPos(-1),
        sortColumn(-1), 
        sortAscendingOrder(false), 
        pinned(false),
        id(-1) {}
    
    SavedSearchQuery(const SavedSearchQuery& other) = default;
    SavedSearchQuery& operator=(const SavedSearchQuery& other) = default;
    bool operator==(const SavedSearchQuery& other) const {
        return other.title == this->title && other.query == this->query;
    }
    
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

class SavedQueriesDAO : public DAO {
  public:
    SavedQueriesDAO(QSqlDatabase& database);
    
    void initialize();
    SavedSearchQuery saveQuery(LibraryFeature* pFeature, SavedSearchQuery sQuery);
    QList<SavedSearchQuery> getSavedQueries(const QString& settingsName) const;
    QList<SavedSearchQuery> getSavedQueries(const LibraryFeature *pFeature) const;
    SavedSearchQuery getSavedQuery(int id) const;
    SavedSearchQuery moveToFirst(LibraryFeature* pFeature, const SavedSearchQuery& sQuery);
    SavedSearchQuery moveToFirst(LibraryFeature* pFeature, int id);
    bool updateSavedQuery(const SavedSearchQuery& sQuery);
    bool deleteSavedQuery(int id);
    bool exists(const SavedSearchQuery& sQuery);
    int getQueryId(const SavedSearchQuery& sQuery);
    
  private:
    static QString serializeItems(const QSet<DbId>& items);
    static QSet<DbId> deserializeItems(QString text);
    static SavedSearchQuery valueToQuery(const QSqlQuery& query);
    
    static const QString kSelectStart;
    
    QSqlDatabase& m_database;
};

#endif // SAVEDQUERIESDAO_H
