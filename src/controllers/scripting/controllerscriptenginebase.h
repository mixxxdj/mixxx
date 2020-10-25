#pragma once

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QMessageBox>

#include "controllers/controllerpreset.h"
#include "util/duration.h"

class Controller;
class EvaluationException;

/// ControllerScriptEngineBase manages the JavaScript engine for controller scripts.
/// ControllerScriptModuleEngine implements the current system using JS modules.
/// ControllerScriptEngineLegacy implements the legacy hybrid JS/XML system.
class ControllerScriptEngineBase : public QObject {
    Q_OBJECT
  public:
    explicit ControllerScriptEngineBase(Controller* controller);
    virtual ~ControllerScriptEngineBase() = default;

    virtual bool initialize();

    bool executeFunction(QJSValue functionObject, QJSValueList arguments);

    /// Shows a UI dialog notifying of a script evaluation error.
    /// Precondition: QJSValue.isError() == true
    void showScriptExceptionDialog(QJSValue evaluationResult, bool bFatal = false);
    void throwJSError(const QString& message);

    inline void setTesting(bool testing) {
        m_bTesting = testing;
    };

    bool isTesting() {
        return m_bTesting;
    }

  protected:
    virtual void shutdown();

    void scriptErrorDialog(const QString& detailedError, const QString& key, bool bFatal = false);

    QJSValue wrapArrayBufferCallback(const QJSValue& callback);

    bool m_bDisplayingExceptionDialog;
    QJSEngine* m_pJSEngine;

    Controller* m_pController;

    bool m_bTesting;

  protected slots:
    void reload();

  private:
    QJSValue m_makeArrayBufferWrapperFunction;

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);

    friend class ColorMapperJSProxy;
    friend class ControllerEngineTest;
};
