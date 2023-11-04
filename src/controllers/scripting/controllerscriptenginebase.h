#pragma once

#include <QJSValue>
#include <QMessageBox>
#include <memory>

#include "util/runtimeloggingcategory.h"

class Controller;
class QJSEngine;
class ControllerRuntimeData;
#ifdef MIXXX_USE_QML
class TrackCollectionManager;
#endif

/// ControllerScriptEngineBase manages the JavaScript engine for controller scripts.
/// ControllerScriptModuleEngine implements the current system using JS modules.
/// ControllerScriptEngineLegacy implements the legacy hybrid JS/XML system.
class ControllerScriptEngineBase : public QObject {
    Q_OBJECT
  public:
    explicit ControllerScriptEngineBase(
            Controller* controller, const RuntimeLoggingCategory& logger);
    virtual ~ControllerScriptEngineBase() override = default;

    virtual bool initialize();

    bool executeFunction(QJSValue* pFunctionObject, const QJSValueList& arguments = {});

    /// Shows a UI dialog notifying of a script evaluation error.
    /// Precondition: QJSValue.isError() == true
    void showScriptExceptionDialog(const QJSValue& evaluationResult, bool bFatal = false);
#ifdef MIXXX_USE_QML
    /// Precondition: QML.isValid() == true
    void showQMLExceptionDialog(const QQmlError& evaluationResult, bool bFatal = false);
#endif
    void throwJSError(const QString& message);

    bool willAbortOnWarning() const {
        return m_bAbortOnWarning;
    }

    inline void setTesting(bool testing) {
        m_bTesting = testing;
    };

    bool isTesting() const {
        return m_bTesting;
    }

    void setRuntimeData(std::shared_ptr<ControllerRuntimeData> runtimeData) {
        m_pRuntimeData = std::move(runtimeData);
    }

    std::shared_ptr<ControllerRuntimeData> getRuntimeData() const {
        return m_pRuntimeData;
    }

#ifdef MIXXX_USE_QML
    static void registerTrackCollectionManager(
            std::shared_ptr<TrackCollectionManager> pTrackCollectionManager);
#endif

  protected:
    std::shared_ptr<ControllerRuntimeData> m_pRuntimeData;

    virtual void shutdown();

    void scriptErrorDialog(const QString& detailedError, const QString& key, bool bFatal = false);
    void logOrThrowError(const QString& errorMessage);

#ifdef MIXXX_USE_QML
    inline void setQMLMode(bool qmlFlag) {
        m_bQmlMode = qmlFlag;
    }
    inline void setErrorsAreFatal(bool errorsAreFatal) {
        m_bErrorsAreFatal = errorsAreFatal;
    }

    /// Pause the controller engine's thread. Pause is required by rendering
    /// thread (https://doc.qt.io/qt-6/qquickrendercontrol.html#sync). This
    /// allows childrend classes to control whether or not they can be requested
    /// to pause the Controller thread used a "GUI thread" for onboard screens
    void setCanPause(bool canPause);
#endif

    bool m_bDisplayingExceptionDialog;
#ifdef MIXXX_USE_QML
    bool m_bErrorsAreFatal;
#endif
    std::shared_ptr<QJSEngine> m_pJSEngine;

    Controller* m_pController;
    const RuntimeLoggingCategory m_logger;

    bool m_bAbortOnWarning;

#ifdef MIXXX_USE_QML
    bool m_bQmlMode;
#endif
    bool m_bTesting;

#ifdef MIXXX_USE_QML
  private:
    static inline std::shared_ptr<TrackCollectionManager> s_pTrackCollectionManager;
    QWaitCondition m_isPausedCondition;
    QMutex m_pauseMutex;
    bool m_isPaused;
    bool m_canPause;

  public slots:
    void requestPause();
    void requestResume();

  signals:
    void pauseRequested();
    void paused(bool);
#endif

  protected slots:
    void reload();

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);
#ifdef MIXXX_USE_QML
    void handleQMLErrors(const QList<QQmlError>& qmlErrors);
    void doPause();
#endif

    friend class ColorMapperJSProxy;
};
