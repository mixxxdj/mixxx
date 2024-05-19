#pragma once

#include <QMutex>
#include <QObject>
#include <QWaitCondition>

/// @brief A ControllerEngineThreadControl can be used to orchestrate thread
/// pausing on a ControllerScriptEngineBase. Pausing is required by the
/// rendering thread (https://doc.qt.io/qt-6/qquickrendercontrol.html#sync).
/// This offscreen render thread is used to pause the main "GUI thread" for onboard
/// screens.
/// The Qt documentation isn't completely clear about this, but after
/// testing, it appears that the "GUI main thread" is the thread where the QML
/// engine lives in (also the main thread if we were using a
/// QMLApplication, which isn't the case here)
class ControllerEngineThreadControl : public QObject {
    Q_OBJECT
  public:
    explicit ControllerEngineThreadControl(QObject* parent = nullptr);

  public slots:
    // The following slots may be used by the rendering engine to pause the thread.
    // They must be called from a different thread than ControllerEngineThreadControl's.
    bool pause();
    void resume();

    // Change whether or not it is possible to pause the thread. Should be
    // called from the same thread as ControllerEngineThreadControl.
    void setCanPause(bool canPause);
  private slots:
    // Used to effectively pause the thread. Must be called from the same thread
    // as ControllerEngineThreadControl.
    void doPause();

  signals:
    void pauseRequested();

  private:
    QWaitCondition m_isPausedCondition;
    QMutex m_pauseMutex;
    int m_pauseCount{0};
    bool m_isPaused{false};
    bool m_canPause{false};
};
