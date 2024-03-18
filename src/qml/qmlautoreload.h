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
};

} // namespace qml
} // namespace mixxx
