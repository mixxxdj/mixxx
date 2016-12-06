#include <QString>

#include "library/coverartcache.h"
#include "library/libraryfeature.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "util/stringhelper.h"
#include "widget/wpixmapstore.h"

#include "library/features/mixxxlibrary/mixxxlibrarytreemodel.h"

namespace  {
// This is used since MixxxLibraryTreeModel inherits QAbstractItemModel
// and since the data() method is a const method a class atribute can't be 
// changed so this is a hack to allow using coverarts in the model
// since there's another pointer to the class there's no problem in two classes
// coexisting.
QHash<const MixxxLibraryTreeModel*, QHash<quint16, QModelIndex> > m_hashToIndex;
}

MixxxLibraryTreeModel::MixxxLibraryTreeModel(LibraryFeature* pFeature,
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
    
    TrackDAO& trackDAO(pTrackCollection->getTrackDAO());
    connect(&trackDAO, SIGNAL(forceModelUpdate()), this, SLOT(reloadTree()));
    connect(&trackDAO, SIGNAL(tracksAdded(QSet<TrackId>)), 
            this, SLOT(reloadTree()));
    connect(&trackDAO, SIGNAL(tracksRemoved(QSet<TrackId>)),
            this, SLOT(reloadTree()));
    connect(&trackDAO, SIGNAL(trackChanged(TrackId)),
            this, SLOT(reloadTree()));
    
    m_coverQuery << LIBRARYTABLE_COVERART_HASH
                 << LIBRARYTABLE_COVERART_LOCATION 
                 << LIBRARYTABLE_COVERART_SOURCE
                 << LIBRARYTABLE_COVERART_TYPE;
    
    for (QString& s : m_coverQuery) {
        s.prepend("library.");
    }
    m_coverQuery << "track_locations." + TRACKLOCATIONSTABLE_LOCATION;
    reloadTree();
}


QVariant MixxxLibraryTreeModel::data(const QModelIndex& index, int role) const {
    if (role == AbstractRole::RoleSettings) {
        return m_sortOrder;
    }
    
    
    TreeItem* pTree = static_cast<TreeItem*>(index.internalPointer());
    DEBUG_ASSERT_AND_HANDLE(pTree != nullptr) {
        return TreeItemModel::data(index, role);
    }
    
    if (role == AbstractRole::RoleGroupingLetter) {
        if (pTree == m_pLibraryItem || pTree == m_pSettings) {
            return QChar();
        }
        return TreeItemModel::data(index, role);
    }
    
    if (role == AbstractRole::RoleBreadCrumb) {
        if (pTree == m_pLibraryItem) {
            return m_pFeature->title();
        } else {
            return TreeItemModel::data(index, role);
        }
    }
    
    if (role == AbstractRole::RoleQuery) {
        return getQuery(pTree);
    }
    
    // The decoration role contains the icon in QTreeView
    if (role == Qt::DecorationRole) {
        // Role is decoration role, we need to show the cover art
        const CoverInfo& info = pTree->getCoverInfo();
        
        // Currently we only support this two types of cover info
        if (info.type != CoverInfo::METADATA && info.type != CoverInfo::FILE) {
            return TreeItemModel::data(index, role);
        }
        
        CoverArtCache* pCache = CoverArtCache::instance();
        // Set a maximum size of 32px to not use many cache
        QPixmap pixmap = pCache->requestCover(info, this, 32, false, true);
        
        if (pixmap.isNull()) {
            // The icon is not in the cache so we need to wait until the
            // coverFound slot is called. Since the data function is const
            // and we cannot change that we use m_hashToIndex in an anonymous
            // namespace to store the future value that we will get
            m_hashToIndex[this].insert(info.type, index);
            
            // Return a temporary icon
            return QIcon(":/images/library/cover_default.png");
        } else {
            // Good luck icon found
            return QIcon(pixmap);
        }
    }
    
    return TreeItemModel::data(index, role);
}

bool MixxxLibraryTreeModel::setData(const QModelIndex& index, const QVariant& value, 
                                    int role) {
    if (role == AbstractRole::RoleSettings) {
        m_sortOrder = value.toStringList();
        m_pConfig->set(ConfigKey("[Library]", LIBRARYTREEMODEL_SORT),
                       ConfigValue(m_sortOrder.join(",")));
        return true;
    } else {
        return TreeItemModel::setData(index, value, role);
    }
}

void MixxxLibraryTreeModel::reloadTree() {    
    //qDebug() << "LibraryTreeModel::reloadTracksTree";
    beginResetModel();
    // Create root item
    TreeItem* pRootItem = new TreeItem();
    pRootItem->setLibraryFeature(m_pFeature);

    m_pLibraryItem = new TreeItem(tr("Show all"), "", m_pFeature, pRootItem);
    pRootItem->appendChild(m_pLibraryItem);

    QString groupTitle = tr("Grouping Options (%1)").arg(m_sortOrder.join(" > "));
    m_pSettings = new TreeItem(groupTitle, "", m_pFeature, pRootItem);
    pRootItem->appendChild(m_pSettings);
    
    // Deletes the old root item if the previous root item was not null
    setRootItem(pRootItem);
    createTracksTree();
    endResetModel();
}

void MixxxLibraryTreeModel::coverFound(const QObject* requestor, int requestReference,
                                       const CoverInfo&, QPixmap pixmap, bool fromCache) {
    
    if (requestor == this && !pixmap.isNull() && !fromCache) {
        auto it = m_hashToIndex[this].find(requestReference);
        if (it == m_hashToIndex[this].end()) {
            return;
        }
        
        const QModelIndex& index = *it;
        emit(dataChanged(index, index));
    }
}

QVariant MixxxLibraryTreeModel::getQuery(TreeItem* pTree) const {
    DEBUG_ASSERT_AND_HANDLE(pTree != nullptr) {
        return "";
    }
    
    if (pTree == m_pLibraryItem) {
        return "";
    } else if (pTree == m_pSettings) {
        return "$groupingSettings$";
    }
    
    const QString param("%1:=\"%2\"");
    
    int depth = 0;
    TreeItem* pAux = pTree;
    QStringList result;
    
    // We need to know the depth before doing anything
    while (pAux->parent() != m_pRootItem && pAux->parent() != nullptr) {
        pAux = pAux->parent();
        ++depth;
    }
    
    // Generate the query
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

void MixxxLibraryTreeModel::createTracksTree() {

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
    
    // Sorting is required to create the tree because the tree is sorted and 
    // in order to create a tree with levels it must be sorted too.
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
    
    int treeDepth = columns.size();
    if (treeDepth <= 0) {
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
    
    int treeStartQueryIndex = m_coverQuery.size() + 1;
    QVector<QString> lastUsed(treeDepth);
    QChar lastHeader;
    // We add 1 to the total parents because the first parent is the root item
    // with this we can always use parent[i] to get the parent of the element at
    // depth i and to set the parent we avoid checking that i + 1 < treeDepth
    QVector<TreeItem*> parent(treeDepth + 1, nullptr);
    parent[0] = m_pRootItem;
    
    while (query.next()) {
        for (int i = 0; i < treeDepth; ++i) {
            QString treeItemLabel = query.value(treeStartQueryIndex + i).toString();
            QString dataPath = treeItemLabel;
            
            bool unknown = dataPath.isNull();
            if (unknown) {
                dataPath = "";
                treeItemLabel = tr("Unknown");
            }            
            if (!lastUsed[i].isNull() && dataPath.localeAwareCompare(lastUsed[i]) == 0) {                
                continue;
            }

            if (i == 0 && !unknown) {
                // If a new top level is added all the following levels must be
                // reset 
                lastUsed.fill(QString());
                
                // Check if a header must be added
                QChar c = StringHelper::getFirstCharForGrouping(treeItemLabel);
                if (lastHeader != c) {
                    lastHeader = c;
                    TreeItem* pTree = new TreeItem(lastHeader, lastHeader, 
                                                   m_pFeature, parent[0]);
                    pTree->setDivider(true);
                    parent[0]->appendChild(pTree);
                }   
            }
            
            lastUsed[i] = dataPath;
            
            // We need to create a new item
            TreeItem* pTree = new TreeItem(treeItemLabel, dataPath, 
                                           m_pFeature, parent[i]);
            pTree->setTrackCount(0);
            parent[i]->appendChild(pTree);
            parent[i + 1] = pTree;
            
            // Add coverart info
            if (treeStartQueryIndex + i == iAlbum) {
                addCoverArt(cIndex, query, pTree);
            }
        }
        
        // Set track count
        int val = query.value(0).toInt();
        for (int i = 1; i < treeDepth + 1; ++i) {
            TreeItem* pTree = parent[i];
            if (pTree == nullptr) {
                continue;
            }
            
            pTree->setTrackCount(pTree->getTrackCount() + val);
        }
    }
}

void MixxxLibraryTreeModel::addCoverArt(const MixxxLibraryTreeModel::CoverIndex& index,
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
    pTree->setIcon(QIcon(":/images/library/cover_default.png"));
}
