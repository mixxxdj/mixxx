#pragma once

#include <QJSValue>
#include <QMessageBox>
#include <QMutex>
#include <QQmlError>
#include <QWaitCondition>
#include <memory>

#include "javascriptplayerproxy.h"
#include "mixer/playermanager.h"
#include "util/runtimeloggingcategory.h"
#ifdef MIXXX_USE_QML
#include "controllers/controllerenginethreadcontrol.h"
#endif

class Controller;
class QJSEngine;
class TrackCollectionManager;

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

    QObject* getPlayer(const QString& group);

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

    static void registerPlayerManager(std::shared_ptr<PlayerManager> pPlayerManager);

    static void registerTrackCollectionManager(
            std::shared_ptr<TrackCollectionManager> pTrackCollectionManager);

  signals:
    void beforeShutdown();

  protected:
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

  private:
    static inline std::shared_ptr<PlayerManager> s_pPlayerManager;
    static inline std::shared_ptr<TrackCollectionManager> s_pTrackCollectionManager;

#ifdef MIXXX_USE_QML
  protected:
    /// Pause the GUI main thread. Pause is required by rendering
    /// thread (https://doc.qt.io/qt-6/qquickrendercontrol.html#sync). This
    /// offscreen render thread to pause the main "GUI thread" for onboard
    /// screens
    /// The documentation isn't completely clear about this, but after
    /// testing, it appears that the "GUI main thread" is the thread where the QML
    /// engine leaves in (also the main thread if we were using a
    /// QMLApplication, which isn't the case here)
    ControllerEngineThreadControl m_engineThreadControl;
#endif

  protected slots:
    void reload();

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);
#ifdef MIXXX_USE_QML
    void handleQMLErrors(const QList<QQmlError>& qmlErrors);
#endif

    friend class ColorMapperJSProxy;
    friend class MidiControllerTest;
};
