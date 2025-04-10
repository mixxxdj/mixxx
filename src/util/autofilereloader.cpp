#include "util/autofilereloader.h"

#include <QDebug>
#include <QString>

#include "moc_autofilereloader.cpp"
#include "util/runtimeloggingcategory.h"

AutoFileReloader::~AutoFileReloader() = default;

AutoFileReloader::AutoFileReloader(const RuntimeLoggingCategory& loggingCategory)
        : m_loggingCategory(loggingCategory) {
    connect(&m_fileWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &AutoFileReloader::slotFileChanged);
}

void AutoFileReloader::slotFileChanged(const QString& changedFile) {
    qCDebug(m_loggingCategory) << "File" << changedFile << "has been changed.";
    if (m_pathTriggeringReload.remove(changedFile)) {
        m_fileWatcher.removePath(changedFile);
        emit fileChanged(changedFile);
    }
}
