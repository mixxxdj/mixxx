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
            this, SLOT(reloadTree()));
    connect(&trackDAO, SIGNAL(tracksRemoved(QSet<TrackId>)),
            this, SLOT(reloadTree()));
    connect(&trackDAO, SIGNAL(trackChanged(TrackId)),
            this, SLOT(reloadTree()));

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

void LibraryFolderModel::createTracksTree() {
    if (!m_showFolders) {
        TracksTreeModel::createTracksTree();
        return;
    }

    // Get the Library directories
    QStringList dirs(m_pTrackCollection->getDirectoryDAO().getDirs());

    QString queryStr = "SELECT COUNT(%3),%1 "
                       "FROM track_locations INNER JOIN library ON %3=%4 "
                       "WHERE %2=0 AND %1 LIKE :dir "
                       "GROUP BY %1 "
                       "ORDER BY ";
    queryStr = queryStr.arg(TRACKLOCATIONSTABLE_DIRECTORY,
                            "library." + LIBRARYTABLE_MIXXXDELETED,
                            "library." + LIBRARYTABLE_ID,
                            "track_locations." + TRACKLOCATIONSTABLE_ID);
    queryStr += mixxx::DbConnection::collateLexicographically(TRACKLOCATIONSTABLE_DIRECTORY);

    QSqlQuery query(m_pTrackCollection->database());
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
    QStringList lastUsed;
    QList<TreeItem*> parent;
    parent.append(getRootItem());
    bool first = true;

    while (query.next()) {
        QString location = query.value(1).toString();
        //qDebug() << location;

        // Remove the first / character
        QString dispValue = location.mid(dir.size());
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
            TreeItem* pTree = m_pRootItem->appendChild(dir, path);
            pTree->setDivider(true);
        }

        // Do not add empty items
        if (dispValue.isEmpty()) {
            continue;
        }

        // We always use Qt notation for folders "/"
        QStringList parts = dispValue.split("/");
        int treeDepth = parts.size();
        if (treeDepth > lastUsed.size()) {
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

                TreeItem* pItem = parent[i]->appendChild(val, fullPath);
                pItem->setTrackCount(0);

                parent[i + 1] = pItem;
                lastUsed[i] = val;
            }
        }

        // Set track count
        int val = query.value(0).toInt();
        for (int i = 1; i < treeDepth + 1; ++i) {
            TreeItem* pItem = parent[i];
            if (pItem == nullptr) {
                continue;
            }

            pItem->setTrackCount(pItem->getTrackCount() + val);
        }
    }
}
