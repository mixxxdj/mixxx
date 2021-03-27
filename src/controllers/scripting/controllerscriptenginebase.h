#pragma once

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QMessageBox>
#include <memory>

#include "controllers/legacycontrollermapping.h"
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
    virtual ~ControllerScriptEngineBase() override = default;

    virtual bool initialize();

    bool executeFunction(QJSValue functionObject, const QJSValueList& arguments);

    /// Shows a UI dialog notifying of a script evaluation error.
    /// Precondition: QJSValue.isError() == true
    void showScriptExceptionDialog(const QJSValue& evaluationResult, bool bFatal = false);
    void throwJSError(const QString& message);

    inline void setTesting(bool testing) {
        m_bTesting = testing;
    };

    bool isTesting() const {
        return m_bTesting;
    }

  protected:
    virtual void shutdown();

    void scriptErrorDialog(const QString& detailedError, const QString& key, bool bFatal = false);

    bool m_bDisplayingExceptionDialog;
    std::shared_ptr<QJSEngine> m_pJSEngine;

    Controller* m_pController;

    bool m_bTesting;

  protected slots:
    void reload();

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);

    friend class ColorMapperJSProxy;
};
