#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QQmlAbstractUrlInterceptor>
#include <QSet>

namespace mixxx {
namespace qml {

class QmlAutoReload : public QObject, public QQmlAbstractUrlInterceptor {
    Q_OBJECT
  public:
    explicit QmlAutoReload();

    QUrl intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) override;

  signals:
    void triggered();

  private slots:
    void slotFileChanged(const QString& changedFile);

  public slots:
    void clear();

  private:
    QFileSystemWatcher m_fileWatcher;
    // We use a separate set to keep track of file allowed to trigger a reload.
    // This is to prevent double-reload when a file is updated twice
    // in a row as part of the normal saving process. See note in
    // QFileSystemWatcher::fileChanged documentation.
    // We've identified to saving policy so far:
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
};

} // namespace qml
} // namespace mixxx
