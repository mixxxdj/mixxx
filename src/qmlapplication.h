#pragma once

#include <coreservices.h>
#include <util/cmdlineargs.h>

#include <QApplication>
#include <QFileSystemWatcher>
#include <QQmlApplicationEngine>

namespace mixxx {

class QmlApplication : public QObject {
    Q_OBJECT
  public:
    QmlApplication(
            QApplication* app,
            std::shared_ptr<CoreServices> pCoreServices,
            const CmdlineArgs& args);
    ~QmlApplication() = default;

  public slots:
    void loadQml(const QString& path);

  private:
    std::shared_ptr<CoreServices> m_pCoreServices;
    const CmdlineArgs& m_cmdlineArgs;

    std::unique_ptr<QQmlApplicationEngine> m_pQmlAppEngine;
    QFileSystemWatcher m_qmlFileWatcher;
};

} // namespace mixxx
