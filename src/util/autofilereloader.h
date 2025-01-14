#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QSet>

#include "util/runtimeloggingcategory.h"

class AutoFileReloader : public QObject {
    Q_OBJECT
  public:
    explicit AutoFileReloader(const RuntimeLoggingCategory& loggingCategory);
    ~AutoFileReloader() override;

    void clear() {
        const auto files = m_fileWatcher.files();
        if (!files.isEmpty()) {
            m_fileWatcher.removePaths(files);
        }
    }
    // this is idempotent, so while not particularly efficient it is safe
    // to call this on the same path multiple times.
    void addPath(const QString& path) {
        m_fileWatcher.addPath(path);
        m_pathTriggeringReload.insert(path);
    }

  signals:
    // Note that once the reload was handled, any changed paths need to be added back
    // to the watcher using `addPath`.
    void fileChanged(const QString& filepath);

  private slots:
    void slotFileChanged(const QString& changedFile);

  private:
    QFileSystemWatcher m_fileWatcher;
    // We use a separate set to keep track of file allowed to trigger a reload.
    // This is to prevent double-reload when a file is updated twice
    // in a row as part of the normal saving process. See note in
    // QFileSystemWatcher::fileChanged documentation.
    // We've identified two saving policies so far:
    // 1. Append/Truncate (VSCode)
    //    The program first append the new content to the file (firing a signal,
    //    while keeping the path in the watch list) then subsequent truncate to
    //    move the previous file content (firing a second signal, and also
    //    keeping the path in the watch list)
    // 2. Delete/Rewrite (sed in-place, vim)
    //    The program first delete the file (firing a signal, AND removing the
    //    path from the watch list) then subsequent create/write to the same
    //    file path (not firing a second signal, since it has been removed from
    //    the watch path previously)
    // Using a set allows us to ensure the same single reload behavior across
    // the different policies
    QSet<QString> m_pathTriggeringReload;
    RuntimeLoggingCategory m_loggingCategory;
};
