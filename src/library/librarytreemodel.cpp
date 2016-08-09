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
    
    QString recursive = m_pConfig->getValueString(ConfigKey("[Library]","FolderRecursive"));
    m_folderRecursive = recursive.toInt() == 1;
    
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

QStringList LibraryTreeModel::getSortOrder() {
    return m_sortOrder;
}

void LibraryTreeModel::setFolderRecursive(bool recursive) {
    m_folderRecursive = recursive;
    m_pConfig->set(ConfigKey("[Library]", "FolderRecursive"), 
                   ConfigValue((int)recursive));
}

bool LibraryTreeModel::getFolderRecursive() {
    return m_folderRecursive;
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

QVariant LibraryTreeModel::getQuery(TreeItem* pTree) const {
    DEBUG_ASSERT_AND_HANDLE(pTree != nullptr) {
        return "";
    }
    
    if (pTree == m_pLibraryItem || pTree == m_pFoldersRoot) {
        return "";
    }
    const QString param("%1:=\"%2\"");
    
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
    
    if (isFolderItem) {
        return param.arg("folder", pTree->dataPath().toString());
    }
    
    // Find Artist / Album / Genre query
    int depth = 0;
    pAux = pTree;
    
    // We need to know the depth of the item to apply the filter
    while (pAux->parent() != m_pRootItem && pAux->parent() != nullptr) {
        pAux = pAux->parent();
        ++depth;
    }
    
    // Generate the query
    QStringList result;
    
    pAux = pTree;
    while (depth >= 0) {
        QString value = pAux->dataPath().toString();
        if (pAux->isDivider()) {
            value.append("*");
        }
        
        result << param.arg(m_sortOrder[depth], value);
        pAux = pAux->parent();
        --depth;
    }
    
    return result.join(" ");
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
    
    // Get the Library directories
    QStringList dirs(m_pTrackCollection->getDirectoryDAO().getDirs());
    
    QString queryStr = "SELECT DISTINCT %1 FROM %2 "
                       "WHERE %3=0 AND %1 LIKE :dir "
                       "ORDER BY %1 COLLATE localeAwareCompare";
    queryStr = queryStr.arg(TRACKLOCATIONSTABLE_DIRECTORY,
                            "track_locations",
                            TRACKLOCATIONSTABLE_FSDELETED);
    
    QSqlQuery query(m_pTrackCollection->getDatabase());
    query.prepare(queryStr);
    
    m_pFoldersRoot = new TreeItem(tr("Folders"), "", m_pFeature, m_pRootItem);
    QIcon icon(WPixmapStore::getLibraryIcon(":/images/library/ic_library_folder.png"));
    m_pFoldersRoot->setIcon(icon);
    m_pRootItem->appendChild(m_pFoldersRoot);    
    
    for (const QString& dir : dirs) {
        query.bindValue(":dir", dir + "%");
        
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return;
        }
        
        // For each source folder create the tree
        createTreeFromSource(dir, query);
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

void LibraryTreeModel::createTreeFromSource(const QString& dir, QSqlQuery& query) {
    
    QStringList lastUsed;
    QList<TreeItem*> parent;
    parent.append(m_pFoldersRoot);
    bool first = true;
    
    while (query.next()) {
        QString value = query.value(0).toString();
        qDebug() << value;
        
        
        // Remove the 
        QString dispValue = value.mid(dir.size());
        if (dispValue.startsWith("/")) {
            dispValue = dispValue.mid(1);
        }
        
        // Add a header
        if (first) {
            first = false;
            QString path = dir;
            if (m_folderRecursive) {
                path.append("*");
            }
            TreeItem* pTree = new TreeItem(dir, path, m_pFeature, m_pFoldersRoot);
            pTree->setDivider(true);
            m_pFoldersRoot->appendChild(pTree);
        }

        // Do not add empty items
        if (dispValue.isEmpty()) {    
            continue;
        }
        
        QStringList parts = dispValue.split("/");
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
                
                QString fullPath = dir;
                for (int j = 0; j <= i; ++j) {
                    fullPath += "/" + parts.at(j);
                }
                
                if (m_folderRecursive) {
                    fullPath.append("*");
                }
                
                TreeItem* pItem = new TreeItem(val, fullPath, m_pFeature, parent[i]);
                parent[i]->appendChild(pItem);
                
                parent[i + 1] = pItem;
                lastUsed[i] = val;
            }
        }
    }
}
