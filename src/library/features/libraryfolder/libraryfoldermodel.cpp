#include <QStringList>

#include "library/features/libraryfolder/libraryfoldermodel.h"

#include "library/libraryfeature.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"

LibraryFolderModel::LibraryFolderModel(LibraryFeature* pFeature,
                                       TrackCollection* pTrackCollection,
                                       UserSettingsPointer pConfig,
                                       QObject* parent)
        : TreeItemModel(parent),
          m_pFeature(pFeature),
          m_pTrackCollection(pTrackCollection),
          m_pConfig(pConfig),
          m_pShowAllItem(nullptr) {

    QString recursive = m_pConfig->getValueString(ConfigKey("[Library]",
                                                            "FolderRecursive"));
    m_folderRecursive = recursive.toInt() == 1;
    
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
    if (role == AbstractRole::RoleSettings) {
        m_folderRecursive = value.toBool();
        m_pConfig->set(ConfigKey("[Library]", "FolderRecursive"), 
                       ConfigValue((int)m_folderRecursive));
        return true;
    } else {
        return TreeItemModel::setData(index, value, role);
    }
}

QVariant LibraryFolderModel::data(const QModelIndex& index, int role) const {
    if (role == AbstractRole::RoleSettings) {
        return m_folderRecursive;
    }
    
    TreeItem* pTree = static_cast<TreeItem*>(index.internalPointer());
    DEBUG_ASSERT_AND_HANDLE(pTree != nullptr) {
        return TreeItemModel::data(index, role);
    }
    
    if (role == AbstractRole::RoleBreadCrumb) {
        if (pTree == m_pShowAllItem) {
            return m_pFeature->title();
        } else {
            return TreeItemModel::data(index, role);
        }
    }
    
    if (role == AbstractRole::RoleQuery) {
        // User has clicked the show all item
        if (pTree == m_pShowAllItem) {
            return "";
        }
        
        const QString param("%1:=\"%2\"");
        return param.arg("folder", pTree->dataPath().toString());
    }

    return TreeItemModel::data(index, role);
}

void LibraryFolderModel::reloadTree() {
    //qDebug() <<  "LibraryFolderModel::reloadTree()";
    beginResetModel();
    // Remove current root
    setRootItem(new TreeItem(m_pFeature));

    // Add "show all" item
    m_pShowAllItem = new TreeItem(tr("Show all"), "", m_pFeature, m_pRootItem);
    m_pRootItem->appendChild(m_pShowAllItem);

    // Get the Library directories
    QStringList dirs(m_pTrackCollection->getDirectoryDAO().getDirs());

    QString queryStr = "SELECT COUNT(%3),%1 "
                       "FROM track_locations INNER JOIN library ON %3=%4 "
                       "WHERE %2=0 AND %1 LIKE :dir "
                       "GROUP BY %1 "
                       "ORDER BY %1 COLLATE localeAwareCompare";
    queryStr = queryStr.arg(TRACKLOCATIONSTABLE_DIRECTORY,
                            "library." + LIBRARYTABLE_MIXXXDELETED,
                            "library." + LIBRARYTABLE_ID,
                            "track_locations." + TRACKLOCATIONSTABLE_ID);

    QSqlQuery query(m_pTrackCollection->getDatabase());
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
    endResetModel();
}

void LibraryFolderModel::createTreeForLibraryDir(const QString& dir, QSqlQuery& query) {
    QStringList lastUsed;
    QList<TreeItem*> parent;
    parent.append(m_pRootItem);
    bool first = true;
    
    while (query.next()) {
        QString location = query.value(1).toString();
        //qDebug() << location;
        
        
        // Remove the 
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
            TreeItem* pTree = new TreeItem(dir, path, m_pFeature, m_pRootItem);
            pTree->setDivider(true);
            m_pRootItem->appendChild(pTree);
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
                
                TreeItem* pItem = new TreeItem(val, fullPath, m_pFeature, parent[i]);
                pItem->setTrackCount(0);
                parent[i]->appendChild(pItem);
                
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
