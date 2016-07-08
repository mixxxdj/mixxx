#include <QSqlQuery>

#include "library/librarytreemodel.h"
#include "library/mixxxlibraryfeature.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"

LibraryTreeModel::LibraryTreeModel(MixxxLibraryFeature* pFeature,
                                   TrackCollection* pTrackCollection,
                                   QObject* parent)
        : TreeItemModel(parent),
          m_pFeature(pFeature),
          m_pTrackCollection(pTrackCollection) {
    TreeItem* pRootItem = new TreeItem();
    pRootItem->setLibraryFeature(pFeature);
    QString title = pFeature->title().toString();

    TreeItem* pLibraryChildItem = new TreeItem(title, title, pFeature, pRootItem);

    pRootItem->appendChild(pLibraryChildItem);
    setRootItem(pRootItem);

    // By default sort by Artist -> Album
    // TODO(jmigual) store the sort order in the configuration
    m_sortOrder << LIBRARYTABLE_ARTIST << LIBRARYTABLE_ALBUM;
    createTracksTree();
}

void LibraryTreeModel::createTracksTree() {

    QStringList columns;
    for (const QString& col : m_sortOrder) {
        columns << "library." + col;
    }

    QString queryStr = "SELECT DISTINCT %1 "
                       "FROM library "
                       "WHERE library.%2 != 1 "
                       "ORDER BY %1";
    queryStr = queryStr.arg(columns.join(","), LIBRARYTABLE_MIXXXDELETED);
        

    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare(queryStr);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    
    qDebug() << "LibraryTreeModel::createTracksTree" << query.executedQuery();
    
    TreeItem* parent = m_pRootItem;
    TreeItem* lastInserted = nullptr;
    
    int size = columns.size();
    QVector<QString> lastUsed(size);
    
    int index = 0;
    
}

void LibraryTreeModel::createTreeRecursive(TreeItem* parent,
                                           QVector<QString>& lastInserted, 
                                           int index, QSqlQuery& query) {
    
}
