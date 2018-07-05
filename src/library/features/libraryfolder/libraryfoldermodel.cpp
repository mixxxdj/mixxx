#include <QStringList>

#include "library/features/libraryfolder/libraryfoldermodel.h"
#include "library/dao/trackschema.h"

#include "library/libraryfeature.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"

#include "util/db/dbconnection.h"

LibraryFolderModel::LibraryFolderModel(LibraryFeature* pFeature,
                                       TrackCollection* pTrackCollection,
                                       UserSettingsPointer pConfig,
                                       QObject* parent)
        : TracksTreeModel(pFeature, pTrackCollection, pConfig, parent),
          m_showFolders(false) {

    QString recursive = m_pConfig->getValueString(
            ConfigKey("[Library]", LIBRARYFOLDERMODEL_RECURSIVE));
    m_folderRecursive = recursive.toInt() == 1;

    QString showFolders = m_pConfig->getValueString(
            ConfigKey("[Library]", LIBRARYFOLDERMODEL_FOLDER));
    m_showFolders = showFolders.toInt() == 1;

    TrackDAO& trackDAO(pTrackCollection->getTrackDAO());
    connect(&trackDAO, SIGNAL(forceModelUpdate()), this, SLOT(reloadTree()));
    connect(&trackDAO, SIGNAL(tracksAdded(QSet<TrackId>)),
            this, SLOT(tracksAdded(QSet<TrackId>)));
    connect(&trackDAO, SIGNAL(tracksRemoved(QSet<TrackId>)),
            this, SLOT(tracksRemoved(QSet<TrackId>)));
    connect(&trackDAO, SIGNAL(trackChanged(TrackId)),
            this, SLOT(trackChanged(TrackId)));

    reloadTree();
}

bool LibraryFolderModel::setData(
        const QModelIndex& index, const QVariant& value, int role) {
    if (role == AbstractRole::RoleSorting) {
        QStringList sort = value.toStringList();
        m_showFolders = sort.first() == LIBRARYFOLDERMODEL_FOLDER;
        m_pConfig->set(ConfigKey("[Library]", LIBRARYFOLDERMODEL_FOLDER),
                       ConfigValue((int)m_showFolders));

    } else if (role == AbstractRole::RoleSettings) {
        m_folderRecursive = value.toBool();
        m_pConfig->set(ConfigKey("[Library]", "FolderRecursive"),
                       ConfigValue((int)m_folderRecursive));
        return true;
    }

    return TracksTreeModel::setData(index, value, role);
}

QVariant LibraryFolderModel::data(const QModelIndex& index, int role) const {
    if (role == AbstractRole::RoleSettings) {
        return m_folderRecursive;
    } else if (role == AbstractRole::RoleSorting) {
        if (m_showFolders)
            return LIBRARYFOLDERMODEL_FOLDER;

        return TracksTreeModel::data(index, role);
    } else if (role == AbstractRole::RoleBreadCrumb) {
        return TracksTreeModel::data(index, role);
    }

    TreeItem* pTree = static_cast<TreeItem*>(index.internalPointer());
    VERIFY_OR_DEBUG_ASSERT(pTree != nullptr) {
        return TreeItemModel::data(index, role);
    }

    if (role == AbstractRole::RoleQuery) {
        // User has clicked the show all item or we are showing the library
        // instead of the folders
        if (!m_showFolders || pTree == m_pShowAll || pTree == m_pGrouping) {
            return TracksTreeModel::data(index, role);
        }

        const QString param("%1:=\"%2\"");
        return param.arg("folder", pTree->getData().toString());
    }

    return TracksTreeModel::data(index, role);
}

void LibraryFolderModel::tracksAdded(const QSet<TrackId> trackIds) {
    if (!m_showFolders) {
        TracksTreeModel::tracksAdded(trackIds);
        return;
    }

    beginResetModel();

    QStringList dirs(m_pTrackCollection->getDirectoryDAO().getDirs());
    QSqlQuery query(m_pTrackCollection->database());
    QString queryStr = createQueryStr(true);
    query.prepare(queryStr);

    for (const TrackId& id : trackIds) {
        query.bindValue(":id", id.toVariant());

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return;
        }

        DEBUG_ASSERT(query.next());

        TrackId trackId(query.value(0));
        QString location(query.value(1).toString());

        for (const QString& dir : dirs) {
            if (location.startsWith(dir)) {
                insertTrackToTree(id, dir, location, getRootItem());
                break;
            }
        }
    }

    endResetModel();
}

void LibraryFolderModel::createTracksTree() {
    if (!m_showFolders) {
        TracksTreeModel::createTracksTree();
        return;
    }

    // Get the Library directories
    QStringList dirs(m_pTrackCollection->getDirectoryDAO().getDirs());
    QSqlQuery query(m_pTrackCollection->database());

    QString queryStr = createQueryStr(false);
    query.prepare(queryStr);

    for (const QString& dir : dirs) {
        query.bindValue(":dir", dir + "%");

        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            return;
        }

        // For each source folder create the tree
        createTreeForLibraryDir(dir, query);
    }
}

QString LibraryFolderModel::getGroupingOptions() {
    if (m_showFolders)
        return tr("Folders");

    return TracksTreeModel::getGroupingOptions();
}

void LibraryFolderModel::createTreeForLibraryDir(const QString& dir, QSqlQuery& query) {
    while (query.next()) {
        TrackId trackId(query.value(0));
        QString location(query.value(1).toString());
        insertTrackToTree(trackId,
                          dir, location.mid(dir.size() + 1),
                          getRootItem());
    }
}

QString LibraryFolderModel::createQueryStr(bool singleId) {
    QString queryStr = "SELECT %3,%1 "
                       "FROM track_locations INNER JOIN library ON %3=%4 "
                       "WHERE %2=0 AND ";

    queryStr += singleId ? "%3 = :id" : "%1 LIKE :dir";

    queryStr = queryStr.arg(TRACKLOCATIONSTABLE_DIRECTORY,
                            "library." + LIBRARYTABLE_MIXXXDELETED,
                            "library." + LIBRARYTABLE_ID,
                            "track_locations." + TRACKLOCATIONSTABLE_ID);

    return queryStr;
}

void LibraryFolderModel::insertTrackToTree(TrackId id,
                                           QString dir,
                                           QString location,
                                           TreeItem* pLevel) {
    pLevel->m_childTracks.insert(id);

    QStringList splitted = location.split("/");
    if (splitted.isEmpty() || location.isEmpty()) {
        return;
    }

    QString currentLabel = splitted.takeFirst();
    QString currentPath = dir + "/" + currentLabel;
    QString currentPathTemp = currentPath + (m_folderRecursive ? "*" : "");
    location = splitted.join("/");

    // Search where do we have to insert the next level
    TreeItem* pInsert = nullptr;
    QString lastHeader;
    for (TreeItem* pChild : pLevel->children()) {
        const QString itemPath = pChild->getData().toString();
        const int comparedResult = currentPathTemp.compare(itemPath);

        if (comparedResult == 0) {
            pChild->m_childTracks.insert(id);
            insertTrackToTree(id, currentPath, location, pChild);
            return;
        } else if (comparedResult < 0) {
            pInsert = pChild;
            break;
        }

        if (pChild->isDivider()) {
            lastHeader = itemPath;
        }
    }

    int row = (pInsert == nullptr ?
                   pLevel->childRows() : pInsert->parentRow());

    if (pLevel == getRootItem() && lastHeader != dir) {
        TreeItem* pItem = pLevel->insertChild(row, dir, dir);
        pItem->setDivider(true);
        ++row;
    }

    TreeItem* pItem = pLevel->insertChild(row, currentLabel, currentPathTemp);

    insertTrackToTree(id, currentPath, location, pItem);
}
