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

    TreeItem* pLibraryChildItem = new TreeItem(title, title, m_pFeature, pRootItem);
    pLibraryChildItem->setIcon(m_pFeature->getIcon());

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
    
    int size = columns.size();
    if (size <= 0) {
        return;
    }
    
    QVector<QString> lastUsed(size);
    QChar lastHeader;
    QVector<TreeItem*> parent(size + 1, nullptr);
    parent[0] = m_pRootItem;
    
    while (query.next()) {
        for (int i = 0; i < size; ++i) {
            QString value = query.value(i).toString();
            
            bool uknown = (value == "");
            if (uknown) {
                value = tr("Unknown");
            }            
            if (!lastUsed[i].isNull() && value == lastUsed[i]) {
                continue;
            }
            
            if (i == 0) {
                // If a new top level is added all the following levels must be
                // reset 
                for (QString& s : lastUsed) {
                    s = QString();
                }
                
                // Check if a header must be added
                if (!uknown && lastHeader != value.at(0).toUpper()) {
                    lastHeader = value.at(0).toUpper();
                    TreeItem* pTree = 
                        new TreeItem(lastHeader, lastHeader, m_pFeature, parent[0]);
                    pTree->setDivider(true);
                    parent[0]->appendChild(pTree);
                }
                
            }
            lastUsed[i] = value;
            
            // We need to create a new item
            TreeItem* pTree = new TreeItem(value, value, m_pFeature, parent[i]);
            parent[i]->appendChild(pTree);
            parent[i + 1] = pTree;
        }    
    }
    
    triggerRepaint();
}
