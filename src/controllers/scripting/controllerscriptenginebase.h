#pragma once

#include <QJSValue>
#include <QMessageBox>
#include <memory>

#include "util/runtimeloggingcategory.h"

class Controller;
class QJSEngine;

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

  signals:
    void beforeShutdown();

  protected:
    virtual void shutdown();

    void scriptErrorDialog(const QString& detailedError, const QString& key, bool bFatal = false);
    void logOrThrowError(const QString& errorMessage);

    bool m_bDisplayingExceptionDialog;
    std::shared_ptr<QJSEngine> m_pJSEngine;

    Controller* m_pController;
    const RuntimeLoggingCategory m_logger;

    bool m_bAbortOnWarning;

    bool m_bTesting;

  protected slots:
    void reload();

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);

    friend class ColorMapperJSProxy;
    friend class MidiControllerTest;
};
