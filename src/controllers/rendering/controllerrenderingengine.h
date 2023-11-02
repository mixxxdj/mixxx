#pragma once

#include <QFileSystemWatcher>
#include <QJSValue>
#include <QLabel>
#include <QMutex>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>
#include <bit>

#include "controllers/legacycontrollermapping.h"
#include "controllers/scripting/controllerscriptenginebase.h"
#include "preferences/configobject.h"
#include "util/parented_ptr.h"
#include "util/runtimeloggingcategory.h"
#include "util/time.h"

class Controller;

class ControllerRenderingEngine : public QObject {
    Q_OBJECT
  public:
    ControllerRenderingEngine(const LegacyControllerMapping::ScreenInfo& info);
    ~ControllerRenderingEngine();

    bool event(QEvent* event) override;

    const QSize& size() const {
        return m_screenInfo.size;
    }

    bool isValid() const {
        return m_isValid;
    }

    bool isRunning() const {
        return m_pThread && m_pThread->isRunning();
    }

    QQuickWindow* quickWindow() const {
        return m_quickWindow.get();
    }

    const LegacyControllerMapping::ScreenInfo& info() const {
        return m_screenInfo;
    }

  public slots:
    virtual void requestSend(Controller* controller, const QByteArray& frame);
    void requestSetup(std::shared_ptr<QQmlEngine> qmlEngine);
    void start();
    virtual bool stop();

  private slots:
    void finish();
    void renderFrame();
    void setup(std::shared_ptr<QQmlEngine> qmlEngine);
    void send(Controller* controller, const QByteArray& frame);

  signals:
    void frameRendered(const LegacyControllerMapping::ScreenInfo& screeninfo,
            QImage frame,
            const QDateTime& timestamp);
    void setupRequested(std::shared_ptr<QQmlEngine> engine);
    void stopRequested();
    void sendRequested(Controller* controller, const QByteArray& frame);

  private:
    virtual void prepare();

    mixxx::Duration m_nextFrameStart;

    LegacyControllerMapping::ScreenInfo m_screenInfo;

    std::unique_ptr<QThread> m_pThread;

    std::unique_ptr<QOpenGLContext> m_context;
    std::unique_ptr<QOffscreenSurface> m_offscreenSurface;
    std::unique_ptr<QQuickRenderControl> m_renderControl;
    std::unique_ptr<QQuickWindow> m_quickWindow;

    std::unique_ptr<QOpenGLFramebufferObject> m_fbo;

    GLenum m_GLDataFormat;
    GLenum m_GLDataType;

    bool m_isValid;

    QWaitCondition m_waitCondition;
    QMutex m_mutex;
};
