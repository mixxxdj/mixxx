#pragma once

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QString>
#include <QTimer>
#include <memory>

#include "coreservices.h"
#include "qmlautoreload.h"

class GuiTick;
class VisualsManager;
#if defined(Q_OS_ANDROID)
class QQuickWindow;
class APerformanceHintSession;
#endif

namespace mixxx {
namespace qml {

class QmlApplication : public QObject {
    Q_OBJECT
  public:
    QmlApplication(
            QApplication* app,
            std::shared_ptr<CoreServices> pCoreServices,
            const QString& mainQmlFilePath = QString());
    ~QmlApplication() override;

    bool isReady() const {
        return m_loadSucceeded;
    }

  public slots:
    bool loadQml(const QString& path);

#if defined(Q_OS_ANDROID)
  private slots:
    void slotFrameSwapped();
    void slotWindowChanged(QQuickWindow* window);
#endif

  private:
    std::shared_ptr<CoreServices> m_pCoreServices;
    std::unique_ptr<::VisualsManager> m_visualsManager;
    std::unique_ptr<GuiTick> m_pGuiTick;
    QTimer m_guiTickTimer;

    QString m_mainFilePath;

    std::unique_ptr<QQmlApplicationEngine> m_pAppEngine;
    bool m_loadSucceeded;
    QmlAutoReload m_autoReload;

#if defined(Q_OS_ANDROID)
    // The following are necessary to implement the performance manager
    // (https://developer.android.com/ndk/reference/group/a-performance-hint)
    // This helps the system automatically adjust performance and prevent frame
    // drops
    PerformanceTimer m_frameTimer;
    APerformanceHintSession* m_perfSession;
#endif
};

} // namespace qml
} // namespace mixxx
