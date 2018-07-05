#include <QString>

#include "library/dao/trackschema.h"
#include "library/coverartcache.h"
#include "library/libraryfeature.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "util/stringhelper.h"
#include "widget/wpixmapstore.h"

#include "library/features/tracks/trackstreemodel.h"
#include "util/db/dbconnection.h"

namespace  {
// This is used since MixxxLibraryTreeModel inherits QAbstractItemModel
// and since the data() method is a const method a class atribute can't be
// changed so this is a hack to allow using coverarts in the model
// since there's another pointer to the class there's no problem in two classes
// coexisting.
QHash<const TracksTreeModel*, QHash<quint16, QModelIndex> > m_hashToIndex;
}

TracksTreeModel::TracksTreeModel(LibraryFeature* pFeature,
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
            this, SLOT(tracksAdded(QSet<TrackId>)));
    connect(&trackDAO, SIGNAL(tracksRemoved(QSet<TrackId>)),
            this, SLOT(tracksRemoved(QSet<TrackId>)));
    connect(&trackDAO, SIGNAL(trackChanged(TrackId)),
            this, SLOT(trackChanged(TrackId)));

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


QVariant TracksTreeModel::data(const QModelIndex& index, int role) const {
    if (role == AbstractRole::RoleSorting) {
        return m_sortOrder;
    }


    TreeItem* pTree = static_cast<TreeItem*>(index.internalPointer());
    VERIFY_OR_DEBUG_ASSERT(pTree != nullptr) {
        return TreeItemModel::data(index, role);
    }

    if (role == AbstractRole::RoleGroupingLetter) {
        if (pTree == m_pShowAll || pTree == m_pGrouping) {
            return QChar();
        }
        return TreeItemModel::data(index, role);
    }

    if (role == AbstractRole::RoleBreadCrumb) {
        if (pTree == m_pShowAll) {
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
            return QIcon(":/images/library/cover_default.svg");
        } else {
            // Good luck icon found
            return QIcon(pixmap);
        }
    }

    return TreeItemModel::data(index, role);
}

bool TracksTreeModel::setData(const QModelIndex& index, const QVariant& value,
                                    int role) {
    if (role == AbstractRole::RoleSorting) {
        m_sortOrder = value.toStringList();
        m_pConfig->set(ConfigKey("[Library]", LIBRARYTREEMODEL_SORT),
                       ConfigValue(m_sortOrder.join(",")));
        return true;
    } else {
        return TreeItemModel::setData(index, value, role);
    }
}

void TracksTreeModel::reloadTree() {
    //qDebug() << "LibraryTreeModel::reloadTracksTree";
    beginResetModel();
    // Create root item
    // Deletes the old root item if the previous root item was not null
    TreeItem* pRootItem = setRootItem(std::make_unique<TreeItem>(m_pFeature));
    createTracksTree();

    QString groupTitle = tr("Grouping Options (%1)").arg(getGroupingOptions());
    m_pGrouping = parented_ptr<TreeItem>(pRootItem->insertChild(0, groupTitle, ""));
    m_pShowAll = parented_ptr<TreeItem>(pRootItem->insertChild(0, tr("Show all"), ""));

    endResetModel();
}

void TracksTreeModel::tracksAdded(const QSet<TrackId> trackIds) {
    beginResetModel();

    // Get the tracks information
    QString queryStr = createQueryStr(true);

    QSqlQuery query(m_pTrackCollection->database());
    query.prepare(queryStr);

    // Since QT does not allow to bind value for " IN (...) " queries
    // we prepare the query and execute it once for each element
    for (const TrackId& id : trackIds) {
        query.bindValue(":id", id.toVariant());

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return;
        }

        DEBUG_ASSERT(query.next());
        insertTrackToTree(query);
    }

    endResetModel();
}

void TracksTreeModel::tracksRemoved(const QSet<TrackId> trackIds) {
    // Remove recursively starting from the root item
    beginResetModel();
    TreeItem* pRoot = getRootItem();
    this->removeTracksRecursive(trackIds, pRoot);
    endResetModel();
}

void TracksTreeModel::trackChanged(TrackId trackId) {
    QSet<TrackId> trackIds{trackId};

    tracksRemoved(trackIds);
    tracksAdded(trackIds);
}

void TracksTreeModel::coverFound(const QObject* requestor, int requestReference,
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

QVariant TracksTreeModel::getQuery(TreeItem* pTree) const {
    VERIFY_OR_DEBUG_ASSERT(pTree != nullptr) {
        return "";
    }

    if (pTree == m_pShowAll) {
        return "";
    } else if (pTree == m_pGrouping) {
        return "$groupingSettings$";
    }

    const QString param("%1:=\"%2\"");

    int depth = 0;
    TreeItem* pAux = pTree;
    QStringList result;

    // We need to know the depth before doing anything
    while (pAux->parent() != getRootItem() && pAux->parent() != nullptr) {
        pAux = pAux->parent();
        ++depth;
    }

    // Generate the query
    pAux = pTree;
    while (depth >= 0) {
        QString value = pAux->getData().toString();
        if (pAux->isDivider()) {
            value.append("*");
        }
        result << param.arg(m_sortOrder[depth], value);

        pAux = pAux->parent();
        --depth;
    }

    return result.join(" ");
}

void TracksTreeModel::createTracksTree() {
    QString queryStr = createQueryStr(false);
    QSqlQuery query(m_pTrackCollection->database());
    query.prepare(queryStr);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }
    //qDebug() << "TracksTreeModel::createTracksTree" << query.executedQuery();

    while (query.next()) {
        insertTrackToTree(query);
    }
}

QString TracksTreeModel::getGroupingOptions() {
    return m_sortOrder.join(" > ");
}

void TracksTreeModel::addCoverArt(const TracksTreeModel::CoverIndex& index,
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
    pTree->setIcon(QIcon(":/images/library/cover_default.svg"));
}

bool TracksTreeModel::removeTracksRecursive(const QSet<TrackId>& trackIds,
                                            TreeItem* pTree) {

    // Stopping condition, no further actions are required if the
    // ids that we are looking for are not in this tree
    if (!pTree->m_childTracks.intersects(trackIds)) {
        return false;
    }

    pTree->m_childTracks.subtract(trackIds);
    if (pTree->m_childTracks.size() <= 0) {
        // If there are no  child tracks then this node should be removed
        // we know that we should remove this node because it intersects
        // with the new tracks Ids

        return true;
    }

    // If the element does not have any children this will return here
    QList<TreeItem*> childsToRemove;
    for (TreeItem* pItem : pTree->children()) {
        bool remove = removeTracksRecursive(trackIds, pItem);

        if (remove) {
            childsToRemove.append(pItem);
        }
    }

    // Since the row changes every time an element is removed
    // we must remove the items one by one after knowing which ones
    // should be deleted
    for (TreeItem* pItem : childsToRemove) {
        pTree->removeChild(pItem->parentRow());
    }


    return false;
}

QString TracksTreeModel::createQueryStr(bool singleId) {
    QStringList columns, sortColumns;
    for (const QString& col : m_sortOrder) {
        columns << "library." + col;
        sortColumns << mixxx::DbConnection::collateLexicographically(col);
    }

    QString queryStr = "SELECT %3,%1,%2 "
                       "FROM library LEFT JOIN track_locations "
                       "ON (%3 = %4) "
                       "WHERE %5 != 1 AND %6 != 1 ";

    if (singleId) {
        queryStr += "AND %3 == :id ";
    }

    queryStr = queryStr.arg(m_coverQuery.join(","),
                            columns.join(","),
                            "library." + LIBRARYTABLE_ID,
                            "track_locations." + TRACKLOCATIONSTABLE_ID,
                            "library." + LIBRARYTABLE_MIXXXDELETED,
                            "track_locations." + TRACKLOCATIONSTABLE_FSDELETED);

    return queryStr;
}

void TracksTreeModel::insertTrackToTree(const QSqlQuery& query) {
    const int treeDepth = m_sortOrder.size();
    const int treeStartQueryIndex = m_coverQuery.size() + 1;
    const TrackId currentId(query.value(0));
    const QSqlRecord record = query.record();
    const int iAlbum = record.indexOf(LIBRARYTABLE_ALBUM);
    const CoverIndex cIndex(record);

    TreeItem* pCurrentLevel = getRootItem();
    pCurrentLevel->m_childTracks.insert(currentId);

    for (int i = 0; i < treeDepth; ++i) {
        QString treeItemLabel = query.value(treeStartQueryIndex + i).toString();
        QString dataPath = treeItemLabel;

        // Treat unknown labels differently
        bool unknown = dataPath.isEmpty();
        if (unknown) {
            dataPath = "";
            treeItemLabel = tr("Unknown");
        }

        // Find the position where the new item must be inserted
        TreeItem* pInsert = nullptr;
        bool found = false;
        QChar lastHeader;
        for (TreeItem* pItem : pCurrentLevel->children()) {
            QString itemPath = pItem->getData().toString();
            int compareResult = dataPath.localeAwareCompare(itemPath);

            if (compareResult == 0) {
                found = true;
                pInsert = pItem;
                break;
            } else if (compareResult < 0) {
                // If we reach this part this means that the item should
                // be inserted here since dataPath < itemPath
                pInsert = pItem;
                break;
            }

            if (pItem->isDivider()) {
                lastHeader = StringHelper::getFirstCharForGrouping(
                        pItem->getLabel());
            }
        }

        if (found) {
            pInsert->m_childTracks.insert(currentId);
            pCurrentLevel = pInsert;
            continue;
        }

        // We need to create a new item
        int row = (pInsert == nullptr ?
                       pCurrentLevel->childRows() : pInsert->parentRow());

        if (i == 0 && !unknown) {
            // Check if a header must be added
            QChar c = StringHelper::getFirstCharForGrouping(treeItemLabel);
            if (lastHeader != c) {
                lastHeader = c;
                TreeItem* pTree =
                    pCurrentLevel->insertChild(row, lastHeader, lastHeader);
                pTree->setDivider(true);
                ++row;
            }
        }

        TreeItem* pTree = pCurrentLevel->insertChild(row, treeItemLabel,
                                                     dataPath);
        pTree->m_childTracks.insert(currentId);

        // Add coverart info, the coverart must only be added on the
        // Album nodes
        if (treeStartQueryIndex + i == iAlbum) {
            addCoverArt(cIndex, query, pTree);
        }

        pCurrentLevel = pTree;
    }
}

TracksTreeModel::CoverIndex::CoverIndex(const QSqlRecord& record) {
    iCoverHash = record.indexOf(LIBRARYTABLE_COVERART_HASH);
    iCoverLoc = record.indexOf(LIBRARYTABLE_COVERART_LOCATION);
    iCoverSrc = record.indexOf(LIBRARYTABLE_COVERART_SOURCE);
    iCoverType = record.indexOf(LIBRARYTABLE_COVERART_TYPE);
    iTrackLoc = record.indexOf(TRACKLOCATIONSTABLE_LOCATION);
}
