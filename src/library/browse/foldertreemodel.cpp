#include <filesystem>

#include <QFileInfo>
#include <QtConcurrent>

#include "library/browse/browsefeature.h"
#include "library/browse/foldertreemodel.h"
#include "library/treeitem.h"
#include "moc_foldertreemodel.cpp"

FolderTreeModel::FolderTreeModel(QObject *parent)
        : TreeItemModel(parent) {
    QObject::connect(&m_fsWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(showModified(QString)));
    m_pool.setMaxThreadCount(5);
}

FolderTreeModel::~FolderTreeModel() {
}

/* A tree model of the filesystem should be initialized lazy.
 * It will take the universe to iterate over all files over filesystem
 * hasChildren() returns true if a folder has subfolders although
 * we do not know the precise number of subfolders.
 *
 * Note that BrowseFeature inserts folder trees dynamically and rowCount()
 * is only called if necessary.
 */
bool FolderTreeModel::hasChildren(const QModelIndex& parent) const {
    TreeItem *item = static_cast<TreeItem*>(parent.internalPointer());
    /* Usually the child count is 0 because we do lazy initialization
     * However, for, buid-in items such as 'Quick Links' there exist
     * child items at init time
     */
    if (item->getData().toString() == QUICK_LINK_NODE) {
        return true;
    }
    //Can only happen on Windows
    if (item->getData().toString() == DEVICE_NODE) {
        return true;
    }

    // In all other cases the getData() points to a folder
    QString folder = item->getData().toString();
    return checkFS(folder);
}

void FolderTreeModel::directoryModified(const QString& str) {
    if (m_directoryCache.count(str)) {
        m_directoryCache.erase(str);
    }
}

void FolderTreeModel::insertTreeItemRows(QList<TreeItem*> &rows, int position, const QModelIndex& parent) {
    if (rows.isEmpty()) {
        return;
    }
    QFutureSynchronizer<void> sync;
    foreach(const TreeItem* row, rows) {
        // init cache
        sync.addFuture(QtConcurrent::run(&m_pool, [=]() {
            auto absolutePath = row->getData().toString();
            this->checkFS(absolutePath);
        }));
    }
    sync.waitForFinished();
    TreeItemModel::insertTreeItemRows(rows, position, parent);
}

bool FolderTreeModel::checkFS(const QString& path) const { 
    const auto it = m_directoryCache.find(path);
    if (it != m_directoryCache.end()) {
        return it->second;
    }
    std::filesystem::path fsPath(path.toStdString());
    try {
        for (const auto& c: std::filesystem::directory_iterator(fsPath)) {
                if (c.is_directory()) {
                    m_directoryCache.emplace(path, true);
                    return true;
                }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        qDebug() << e.what();
    }
    m_directoryCache.emplace(path, false);
    return false;
}
