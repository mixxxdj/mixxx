#pragma once

#include <coreservices.h>

#include <QApplication>
#include <QFileSystemWatcher>
#include <QQmlApplicationEngine>

namespace mixxx {
namespace skin {
namespace qml {

class QmlApplication : public QObject {
    Q_OBJECT
  public:
    QmlApplication(
            QApplication* app,
            std::shared_ptr<CoreServices> pCoreServices);
    ~QmlApplication() override = default;

  public slots:
    void loadQml(const QString& path);

  private:
    std::shared_ptr<CoreServices> m_pCoreServices;

    QString m_mainFilePath;

    std::unique_ptr<QQmlApplicationEngine> m_pAppEngine;
    QFileSystemWatcher m_fileWatcher;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
