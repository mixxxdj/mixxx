#include <QSqlQuery>

#include "library/coverartcache.h"
#include "library/librarytreemodel.h"
#include "library/mixxxlibraryfeature.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"

namespace  {
QHash<quint16, QModelIndex> m_hashToIndex;
}

LibraryTreeModel::LibraryTreeModel(MixxxLibraryFeature* pFeature,
                                   TrackCollection* pTrackCollection,
                                   QObject* parent)
        : TreeItemModel(parent),
          m_pFeature(pFeature),
          m_pTrackCollection(pTrackCollection) {
    
    // By default sort by Artist -> Album
    // TODO(jmigual) store the sort order in the configuration
    m_sortOrder << LIBRARYTABLE_ARTIST << LIBRARYTABLE_ALBUM;
    
    m_coverQuery << LIBRARYTABLE_COVERART_HASH
                 << LIBRARYTABLE_COVERART_LOCATION 
                 << LIBRARYTABLE_COVERART_SOURCE
                 << LIBRARYTABLE_COVERART_TYPE;
    
    for (QString& s : m_coverQuery) {
        s.prepend("library.");
    }
    m_coverQuery << "track_locations." + TRACKLOCATIONSTABLE_LOCATION;
    
    reloadTracksTree();
}

QVariant LibraryTreeModel::data(const QModelIndex& index, int role) const {
    
    // The decoration role contains the icon in QTreeView
    if (role != Qt::DecorationRole) {
        return TreeItemModel::data(index, role);
    }
    
    TreeItem* pTree = static_cast<TreeItem*>(index.internalPointer());
    DEBUG_ASSERT_AND_HANDLE(pTree != nullptr) {
        return QVariant();
    }
    
    // The library item has its own icon
    if (pTree == m_pLibraryItem) {
        return pTree->getIcon();
    }
    
    const CoverInfo& info = pTree->getCoverInfo();
    
    // Currently we only support this two types of cover info
    if (info.type != CoverInfo::METADATA && info.type != CoverInfo::FILE) {
        return QVariant();
    }
    
    CoverArtCache* pCache = CoverArtCache::instance();
    // Set a maximum size of 64px to not use many cache
    QPixmap pixmap = pCache->requestCover(info, this, info.hash, 64);
    
    if (pixmap.isNull()) {
        // The icon is not in the cache so we need to wait until the
        // coverFound slot is called. Since the data function is const
        // and we cannot change that we use m_hashToIndex in an anonymous
        // namespace to store the future value that we will get
        m_hashToIndex.insert(info.type, index);
    } else {
        // Good luck icon found
        return QIcon(pixmap);
    }
    return QVariant();
}

void LibraryTreeModel::setSortOrder(QStringList sortOrder) {
    m_sortOrder = sortOrder;
}

QString LibraryTreeModel::getQuery(TreeItem* pTree) const {
    DEBUG_ASSERT_AND_HANDLE(pTree != nullptr) {
        return "";
    }
    
    if (pTree == m_pLibraryItem) {
        return "";
    }
    
    int depth = 0;
    TreeItem* pAux = pTree;
    
    // We need to know the depth of the item to apply the filter
    while (pAux->parent() != m_pRootItem && pAux->parent() != nullptr) {
        pAux = pAux->parent();
        ++depth;
    }
    
    // Generate the query
    QStringList result;
    pAux = pTree;
    while (depth >= 0) {
        QString value;
        if (pAux->dataPath().toString() == "") {
            value = "";
        } else {
            value = pAux->data().toString();
        }
        
        result << m_sortOrder[depth] % ":\"" % value % "\"";
        pAux = pAux->parent();
        --depth;
    }
    
    return result.join(" ");
}

void LibraryTreeModel::reloadTracksTree() {    
    // Create root item
    TreeItem* pRootItem = new TreeItem();
    pRootItem->setLibraryFeature(m_pFeature);
    QString title = m_pFeature->title().toString();

    m_pLibraryItem = new TreeItem(title, title, m_pFeature, pRootItem);
    m_pLibraryItem->setIcon(m_pFeature->getIcon());

    pRootItem->appendChild(m_pLibraryItem);
    
    // Deletes the old root item if the previous root item was not null
    setRootItem(pRootItem);
    createTracksTree();
}

void LibraryTreeModel::coverFound(const QObject* requestor, int requestReference,
                                  const CoverInfo&, QPixmap pixmap, bool fromCache) {
    
    if (requestor == this && !pixmap.isNull() && !fromCache) {
        auto it = m_hashToIndex.find(requestReference);
        if (it == m_hashToIndex.end()) {
            return;
        }
        
        const QModelIndex& index = *it;
        emit(dataChanged(index, index));
    }
}

void LibraryTreeModel::createTracksTree() {

    QStringList columns;
    for (const QString& col : m_sortOrder) {
        columns << "library." + col;
    }

    QString queryStr = "SELECT DISTINCT %1,%2 "
                       "FROM library LEFT JOIN track_locations "
                       "ON (%3 = %4) "
                       "WHERE %5 != 1 "
                       "GROUP BY %2 "
                       "ORDER BY %2";
    queryStr = queryStr.arg(m_coverQuery.join(","), 
                            columns.join(","), 
                            "library." + LIBRARYTABLE_ID,
                            "track_locations." + TRACKLOCATIONSTABLE_ID,
                            "library." + LIBRARYTABLE_MIXXXDELETED);
        

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
    QSqlRecord record = query.record();
    
    int iAlbum = record.indexOf(LIBRARYTABLE_ALBUM);
    int iCoverHash = record.indexOf(LIBRARYTABLE_COVERART_HASH);
    int iCoverLoc = record.indexOf(LIBRARYTABLE_COVERART_LOCATION);
    int iCoverSrc = record.indexOf(LIBRARYTABLE_COVERART_SOURCE);
    int iCoverType = record.indexOf(LIBRARYTABLE_COVERART_TYPE);
    int iTrackLoc = record.indexOf(TRACKLOCATIONSTABLE_LOCATION);
    
    for (int i = 0; i < record.count(); ++i) {
        qDebug() << record.fieldName(i);
    }
    
    int extraSize = m_coverQuery.size();
    QVector<QString> lastUsed(size);
    QChar lastHeader;
    QVector<TreeItem*> parent(size + 1, nullptr);
    parent[0] = m_pRootItem;
    
    while (query.next()) {
        for (int i = 0; i < size; ++i) {
            QString value = query.value(extraSize + i).toString();
            QString valueP = value;
            
            bool unknown = (value == "");
            if (unknown) {
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
                if (!unknown && lastHeader != value.at(0).toUpper()) {
                    lastHeader = value.at(0).toUpper();
                    TreeItem* pTree = new TreeItem(parent[0]);
                    pTree->setLibraryFeature(m_pFeature);
                    pTree->setData(lastHeader, lastHeader);                    
                    pTree->setDivider(true);
                    
                    parent[0]->appendChild(pTree);
                }   
            }
            lastUsed[i] = value;
            
            // We need to create a new item
            TreeItem* pTree = new TreeItem(value, valueP, m_pFeature, parent[i]);
            parent[i]->appendChild(pTree);
            parent[i + 1] = pTree;
            
            if (extraSize + i == iAlbum) {
                CoverInfo c;
                c.hash = query.value(iCoverHash).toInt();
                c.coverLocation = query.value(iCoverLoc).toString();
                c.trackLocation = query.value(iTrackLoc).toString();
                
                quint16 source = query.value(iCoverSrc).toInt();
                quint16 type = query.value(iCoverType).toInt();
                c.source = static_cast<CoverInfo::Source>(source);
                c.type = static_cast<CoverInfo::Type>(type);
                pTree->setCoverInfo(c);
            }
        }    
    }
    
    triggerRepaint();
}
