#if defined(__WINDOWS__)
#include <windows.h>
#include <Shellapi.h>
#include <Shlobj.h>
#else
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <QFileInfo>
#include <QtConcurrent>

#include "library/browse/browsefeature.h"
#include "library/browse/foldertreemodel.h"
#include "library/treeitem.h"
#include "moc_foldertreemodel.cpp"
#include "util/qt.h"

FolderTreeModel::FolderTreeModel(QObject* parent)
        : TreeItemModel(parent), m_isRunning(true) {
    connect(&m_fsWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &FolderTreeModel::dirModified);
    connect(&m_fsWatcher,
            &QFileSystemWatcher::directoryChanged,
            this,
            &FolderTreeModel::dirModified);
    connect(this,
            &FolderTreeModel::newChildren,
            this,
            &FolderTreeModel::addChildren,
            Qt::QueuedConnection);
    connect(this,
            &FolderTreeModel::hasSubDirectory,
            this,
            &FolderTreeModel::onHasSubDirectory);

    m_pool.setMaxThreadCount(4);
    QtConcurrent::run(&m_pool, [&]() {
        while (m_isRunning.load(std::memory_order_consume)) {
            if (!m_folderQueue.isEmpty()) {
                m_queueLock.lock();
                const auto& [parent, path] = m_folderQueue.dequeue();
                m_queueLock.unlock();

                m_cacheLock.lockForWrite();
                const auto dirAccess = mixxx::FileAccess(mixxx::FileInfo(path));
                QFileInfoList all = dirAccess.info().toQDir().entryInfoList(
                        QDir::Dirs | QDir::NoDotAndDotDot);

                auto* folders = new QList<TreeItem*>();
                QFutureSynchronizer<void> sync;
                // loop through all the item and construct the children
                foreach (QFileInfo one, all) {
#if defined(__APPLE__)
                    if (one.isDir() && one.fileName().endsWith(".app")) {
                        continue;
                    }
#endif
                    // We here create new items for the sidebar models
                    // Once the items are added to the TreeItemModel,
                    // the models takes ownership of them and ensures their deletion
                    const auto& absPath = one.absoluteFilePath();
                    auto* folder =
                            new TreeItem(one.fileName(), QVariant(absPath));
                    // init cache
                    if (m_directoryCache.find(absPath) ==
                            m_directoryCache.end()) {
                        m_fsWatcher.addPath(absPath);
                        sync.addFuture(
                                QtConcurrent::run(&m_pool, [this, absPath]() {
                                    this->directoryHasChildren(absPath);
                                }));
                    }
                    folders->push_back(folder);
                }
                if (!folders->isEmpty()) {
                    sync.waitForFinished();
                    m_cacheLock.unlock();
                    emit newChildren(parent, folders);
                } else {
                    m_cacheLock.unlock();
                }
            }
            QThread::msleep(100);
        }
    });
}

FolderTreeModel::~FolderTreeModel() {
    m_isRunning.store(false, std::memory_order_release);
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
    TreeItem* item = static_cast<TreeItem*>(parent.internalPointer());
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
    if (m_cacheLock.tryLockForRead()) {
        const auto it = m_directoryCache.find(folder);
        m_cacheLock.unlock();
        if (it != m_directoryCache.end()) {
            return it->second;
        }
        emit mixxx::thisAsNonConst(this)->hasSubDirectory(folder);
    }
    return true;
}

void FolderTreeModel::directoryHasChildren(const QString& path) {
    // Acquire a security token for the path.
    const auto dirAccess = mixxx::FileAccess(mixxx::FileInfo(path));

    /*
     *  The following code is too expensive, general and SLOW since
     *  QDIR::EntryInfoList returns a full QFileInfolist
     *
     *
     *  QDir dir(item->getData().toString());
     *  QFileInfoList all = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
     *  return (all.count() > 0);
     *
     *  We can benefit from low-level filesystem APIs, i.e.,
     *  Windows API or SystemCalls
     */

    bool has_children = false;

#if defined(__WINDOWS__)
    QString folder = path;
    folder.replace("/", "\\");

    //quick subfolder test
    SHFILEINFOW sfi;
    SHGetFileInfo((LPCWSTR)folder.constData(),
            NULL,
            &sfi,
            sizeof(sfi),
            SHGFI_ATTRIBUTES);
    has_children = (sfi.dwAttributes & SFGAO_HASSUBFOLDER);
#else
    // For OS X and Linux
    // http://stackoverflow.com/questions/2579948/checking-if-subfolders-exist-linux

    std::string dot("."), dotdot("..");
    QByteArray ba = QFile::encodeName(path);
    DIR* directory = opendir(ba);
    int unknown_count = 0;
    int total_count = 0;
    if (directory != nullptr) {
        struct dirent* entry;
        while (!has_children && ((entry = readdir(directory)) != nullptr)) {
            if (entry->d_name != dot && entry->d_name != dotdot) {
                total_count++;
                if (entry->d_type == DT_UNKNOWN) {
                    unknown_count++;
                }
                has_children =
                        (entry->d_type == DT_DIR || entry->d_type == DT_LNK);
            }
        }
        closedir(directory);
    }

    // If all files are of type DH_UNKNOWN then do a costlier analysis to
    // determine if the directory has subdirectories. This affects folders on
    // filesystems that do not fully implement readdir such as JFS.
    if (directory == nullptr ||
            (unknown_count == total_count && total_count > 0)) {
        QDir dir(path);
        QFileInfoList all =
                dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        has_children = all.count() > 0;
    }
#endif

    // Cache the result
    m_directoryCache.emplace(path, has_children);
}

void FolderTreeModel::dirModified(const QString& str) {
    if (m_directoryCache.count(str)) {
        if (!QFileInfo::exists(str)) {
            m_cacheLock.lockForWrite();
            // TODO: when m_directoryCache is a prefix tree
            // make sure that any item in the cache that starts
            // with this path is dropped as well.
            m_directoryCache.erase(str);
            m_cacheLock.unlock();

            // TODO: get the position of the path in the TreeItemModel
            // using TreeItemModel::removeRows cleanup the tree. this will
            // not only make the underlying data consistent
            // but as well trigger a redrow of the tree.
        }
    }
}

void FolderTreeModel::processFolder(
        const QModelIndex& parent, const QString& path) const {
    std::lock_guard<std::mutex> lock(m_queueLock);
    m_folderQueue.enqueue(std::make_pair(parent, path));
}

void FolderTreeModel::addChildren(
        const QModelIndex& parent, TreeItemList children) {
    TreeItemModel::insertTreeItemRows(*children, 0, parent);
    delete children;
}

void FolderTreeModel::onHasSubDirectory(const QString& path) {
    QtConcurrent::run(&m_pool, [this, path]() {
        MWriteLocker lock(&m_cacheLock);
        directoryHasChildren(path);
    });
}
