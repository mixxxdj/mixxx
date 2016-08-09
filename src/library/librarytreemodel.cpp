#include <QString>

#include "library/coverartcache.h"
#include "library/librarytreemodel.h"
#include "library/libraryfeature.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"

#include "util/stringhelper.h"
#include "widget/wpixmapstore.h"

namespace  {
QHash<quint16, QModelIndex> m_hashToIndex;
}

LibraryTreeModel::LibraryTreeModel(LibraryFeature* pFeature,
                                   TrackCollection* pTrackCollection, 
                                   UserSettingsPointer pConfig,
                                   QObject* parent)
        : TreeItemModel(parent),
          m_pFeature(pFeature),
          m_pTrackCollection(pTrackCollection),
          m_pConfig(pConfig) {    
    
    QString sort = m_pConfig->getValueString(ConfigKey("[Library]", LIBRARYTREEMODEL_SORT));
    if (sort.isNull()) {
        // By default sort by Artist -> Album
        m_sortOrder << LIBRARYTABLE_ARTIST << LIBRARYTABLE_ALBUM;
    } else {
        m_sortOrder = sort.split(",");
    }
    
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
    if (role != Qt::DecorationRole && role != TreeItemModel::RoleQuery) {
        return TreeItemModel::data(index, role);
    }
    
    TreeItem* pTree = static_cast<TreeItem*>(index.internalPointer());
    DEBUG_ASSERT_AND_HANDLE(pTree != nullptr) {
        return QVariant();
    }
    if (role == TreeItemModel::RoleQuery) {
        return getQuery(pTree);
    }
    
    // Role is decoration role, we need to show the cover art
    const CoverInfo& info = pTree->getCoverInfo();    
    
    // Currently we only support this two types of cover info
    if (info.type != CoverInfo::METADATA && info.type != CoverInfo::FILE) {
        return TreeItemModel::data(index, role);
    }
    
    CoverArtCache* pCache = CoverArtCache::instance();
    // Set a maximum size of 32px to not use many cache
    QPixmap pixmap = pCache->requestCover(info, this, info.hash, 32);
    
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
    m_pConfig->set(ConfigKey("[Library]", LIBRARYTREEMODEL_SORT),
                   ConfigValue(m_sortOrder.join(",")));
}

void LibraryTreeModel::reloadTracksTree() {    
    //qDebug() << "LibraryTreeModel::reloadTracksTree";
    
    // Create root item
    TreeItem* pRootItem = new TreeItem();
    pRootItem->setLibraryFeature(m_pFeature);
    QString title = m_pFeature->title().toString();

    m_pLibraryItem = new TreeItem(title, "", m_pFeature, pRootItem);
    m_pLibraryItem->setIcon(m_pFeature->getIcon());

    pRootItem->appendChild(m_pLibraryItem);
    
    // Deletes the old root item if the previous root item was not null
    setRootItem(pRootItem);
    createFoldersTree();
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

QString LibraryTreeModel::getQuery(TreeItem* pTree) const {
    DEBUG_ASSERT_AND_HANDLE(pTree != nullptr) {
        return "";
    }
    
    if (pTree == m_pLibraryItem || pTree == m_pFoldersRoot) {
        return "";
    }
    
    // Find for folers root 
    TreeItem* pAux = pTree;
    bool isFolderItem = false;
    while (pAux->parent() != nullptr && pAux->parent() != m_pRootItem) {
        if (pAux->parent() == m_pFoldersRoot) {
            isFolderItem = true;
            break;
        }
        pAux = pAux->parent();
    }
    
    // Find Artist / Album / Genre query
    int depth = 0;
    pAux = pTree;
    
    // We need to know the depth of the item to apply the filter
    while (pAux->parent() != m_pRootItem && pAux->parent() != nullptr) {
        pAux = pAux->parent();
        ++depth;
    }
    
    if (isFolderItem) {
        depth -= 1;
    }
    
    // Generate the query
    QStringList result;
    const QString param("%1:\"%2\"");
    
    pAux = pTree;
    while (depth >= 0) {
        QString value = pAux->dataPath().toString();
        if (isFolderItem) {
            result.insert(0, value);
        } else {
            result << param.arg(m_sortOrder[depth], value);
        }
        pAux = pAux->parent();
        --depth;
    }
    
    QString queryStr = result.join(isFolderItem ? "/" : " ");
    if (isFolderItem) {
        queryStr = param.arg(TRACKLOCATIONSTABLE_LOCATION, queryStr);
    }
    return queryStr;
}

void LibraryTreeModel::createTracksTree() {

    QStringList columns;
    for (const QString& col : m_sortOrder) {
        columns << "library." + col;
    }
    
    QStringList sortColumns;
#ifdef __SQLITE3__
    for (const QString& col : m_sortOrder) {
        sortColumns << col + " COLLATE localeAwareCompare";
    }
#else
    sortColumns = m_sortOrder;
#endif

    QString queryStr = "SELECT COUNT(%3),%1,%2 "
                       "FROM library LEFT JOIN track_locations "
                       "ON (%3 = %4) "
                       "WHERE %5 != 1 AND  %7 != 1 "
                       "GROUP BY %2 "
                       "ORDER BY %6 ";
    queryStr = queryStr.arg(m_coverQuery.join(","), 
                            columns.join(","), 
                            "library." + LIBRARYTABLE_ID,
                            "track_locations." + TRACKLOCATIONSTABLE_ID,
                            "library." + LIBRARYTABLE_MIXXXDELETED,
                            sortColumns.join(","),
                            "track_locations." + TRACKLOCATIONSTABLE_FSDELETED);
        

    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare(queryStr);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }
    //qDebug() << "LibraryTreeModel::createTracksTree" << query.executedQuery();
    
    int size = columns.size();
    if (size <= 0) {
        return;
    }
    QSqlRecord record = query.record();
    
    int iAlbum = record.indexOf(LIBRARYTABLE_ALBUM);
    CoverIndex cIndex;
    cIndex.iCoverHash = record.indexOf(LIBRARYTABLE_COVERART_HASH);
    cIndex.iCoverLoc = record.indexOf(LIBRARYTABLE_COVERART_LOCATION);
    cIndex.iCoverSrc = record.indexOf(LIBRARYTABLE_COVERART_SOURCE);
    cIndex.iCoverType = record.indexOf(LIBRARYTABLE_COVERART_TYPE);
    cIndex.iTrackLoc = record.indexOf(TRACKLOCATIONSTABLE_LOCATION);
    
    int extraSize = m_coverQuery.size() + 1;
    QVector<QString> lastUsed(size);
    QChar lastHeader;
    QVector<TreeItem*> parent(size + 1, nullptr);
    parent[0] = m_pRootItem;
    
    while (query.next()) {
        for (int i = 0; i < size; ++i) {
            QString value = query.value(extraSize + i).toString();
            QString valueP = value;
            
            bool unknown = valueP.isNull();
            if (unknown) {
                valueP = "";
                value = tr("Unknown");
            }            
            if (!lastUsed[i].isNull() && valueP.localeAwareCompare(lastUsed[i]) == 0) {                
                continue;
            }

            if (i == 0 && !unknown) {
                // If a new top level is added all the following levels must be
                // reset 
                lastUsed.fill(QString());
                
                // Check if a header must be added
                QChar c = StringHelper::getFirstCharForGrouping(value);
                if (lastHeader != c) {
                    lastHeader = c;
                    TreeItem* pTree = new TreeItem(lastHeader, lastHeader, 
                                                   m_pFeature, parent[0]);
                    pTree->setDivider(true);
                    parent[0]->appendChild(pTree);
                }   
            }
            
            lastUsed[i] = valueP;
            
            // We need to create a new item
            TreeItem* pTree = new TreeItem(value, valueP, m_pFeature, parent[i]);
            pTree->setTrackCount(0);
            parent[i]->appendChild(pTree);
            parent[i + 1] = pTree;
            
            // Add coverart info
            if (extraSize + i == iAlbum && !unknown) {
                addCoverArt(cIndex, query, pTree);
            }
        }
        
        // Set track count
        int val = query.value(0).toInt();
        for (int i = 1; i < size + 1; ++i) {
            TreeItem* pTree = parent[i];
            if (pTree == nullptr) {
                continue;
            }
            
            pTree->setTrackCount(pTree->getTrackCount() + val);
        }
    }
    
    triggerRepaint();
}

void LibraryTreeModel::createFoldersTree() {
    
    QString queryStr = "SELECT DISTINCT %1 FROM %2 WHERE %3=0";
    queryStr = queryStr.arg(TRACKLOCATIONSTABLE_DIRECTORY,
                            "track_locations",
                            TRACKLOCATIONSTABLE_FSDELETED);
    
    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare(queryStr);
    
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }
        
    // Get the Library directories
    QStringList dirs(m_pTrackCollection->getDirectoryDAO().getDirs());
    
    // Set the folders root item;    
    m_pFoldersRoot = new TreeItem(tr("Folders"), "", m_pFeature, m_pRootItem);
    QIcon icon(WPixmapStore::getLibraryIcon(":/images/library/ic_library_folder.png"));
    m_pFoldersRoot->setIcon(icon);
    m_pRootItem->appendChild(m_pFoldersRoot);    
    
    QStringList tempTree;
    while(query.next()) {        
        QString value = query.value(0).toString();
        for (const QString& s : dirs) {
            if (value.startsWith(s)) {
                value = value.mid(s.size());
                if (value.startsWith("/")) {
                    value = value.mid(1);
                }
                
                break;
            }
        }
        if (!value.isEmpty()) {
            tempTree << value;
        }
    }
    tempTree.removeDuplicates();
    
    // Since the user can define multiple library folders we must sort all the 
    // items to acts as a single big library folder and show the relative paths
    // from the "big" folder
    qSort(tempTree.begin(), tempTree.end(), 
        [](const QString& s1, const QString& s2) -> bool {
            return s1.localeAwareCompare(s2) < 0;
        });
    
    QStringList lastUsed;
    QList<TreeItem*> parent;
    parent.append(m_pFoldersRoot);
    
    for (const QString& value : tempTree) {
        QStringList parts = value.split("/", QString::SkipEmptyParts);
        if (parts.size() > lastUsed.size()) {
            for (int i = lastUsed.size(); i < parts.size(); ++i) {
                lastUsed.append(QString());
                parent.append(nullptr);
            }
        }
        
        bool change = false;
        for (int i = 0; i < parts.size(); ++i) {
            const QString& val = parts.at(i);
            if (change || val != lastUsed.at(i)) {
                change = true;
                
                TreeItem* pItem = new TreeItem(val, val, m_pFeature, parent[i]);
                parent[i]->appendChild(pItem);
                
                parent[i + 1] = pItem;
                lastUsed[i] = val;
            }
        }
    }
}

void LibraryTreeModel::addCoverArt(const LibraryTreeModel::CoverIndex& index,
                                   const QSqlQuery& query, TreeItem* pTree) {
    CoverInfo c;
    c.hash = query.value(index.iCoverHash).toInt();
    c.coverLocation = query.value(index.iCoverLoc).toString();
    c.trackLocation = query.value(index.iTrackLoc).toString();
    
    quint16 source = query.value(index.iCoverSrc).toInt();
    quint16 type = query.value(index.iCoverType).toInt();
    c.source = static_cast<CoverInfo::Source>(source);
    c.type = static_cast<CoverInfo::Type>(type);
    pTree->setCoverInfo(c);
}
