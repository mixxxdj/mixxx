#pragma once

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QMessageBox>
#include <QTimerEvent>
#include <QtScript>

#include "controllers/controllerpreset.h"
#include "controllers/softtakeover.h"
#include "preferences/usersettings.h"
#include "util/alphabetafilter.h"
#include "util/duration.h"

class Controller;
class ControllerScriptHandler;
class ControllerScriptInterface;
class EvaluationException;
class ScriptConnection;

/// ControllerScriptHandler loads and executes script files for controller mappings
class ControllerScriptHandler : public QObject {
    Q_OBJECT
  public:
    ControllerScriptHandler(Controller* controller);
    virtual ~ControllerScriptHandler();

    void handleInput(QByteArray data, mixxx::Duration timestamp);

    bool executeFunction(QJSValue functionObject, QJSValueList arguments);
    bool executeFunction(QJSValue functionObject, const QByteArray& data);

    /// Wrap a string of JS code in an anonymous function. This allows any JS
    /// string that evaluates to a function to be used in MIDI mapping XML files
    /// and ensures the function is executed with the correct 'this' object.
    QJSValue wrapFunctionCode(const QString& codeSnippet, int numberOfArgs);

    /// Look up registered script function prefixes
    const QList<QString>& getScriptFunctionPrefixes() {
        return m_scriptFunctionPrefixes;
    };

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

    QJSEngine* scriptEngine() {
        return m_pScriptEngine;
    }

  public slots:
    void loadModule(QFileInfo moduleFileInfo);
    bool loadScriptFiles(const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void initializeScripts(const QList<ControllerPreset::ScriptFileInfo>& scripts);
    void gracefulShutdown();
    void scriptHasChanged(const QString&);

  private slots:
    void errorDialogButton(const QString& key, QMessageBox::StandardButton button);

  private:
    bool evaluateScriptFile(const QFileInfo& scriptFile);
    void initializeScriptEngine();
    void uninitializeScriptEngine();
    void reloadScripts();

    void scriptErrorDialog(const QString& detailedError, const QString& key, bool bFatal = false);
    void generateScriptFunctions(const QString& code);

    bool callFunctionOnObjects(QList<QString>,
            const QString&,
            QJSValueList args = QJSValueList(),
            bool bFatalError = false);
    /// Convert a byteArray to a JS typed array over an ArrayBuffer
    QJSValue byteArrayToScriptValue(const QByteArray& byteArray);
    QJSValue evaluateCodeString(const QString& program, const QString& fileName = QString(), int lineNumber = 1);

    bool m_bDisplayingExceptionDialog;
    QJSEngine* m_pScriptEngine;

    Controller* m_pController;
    QJSValue m_handleInputFunction;
    QJSValue m_shutdownFunction;
    QList<QString> m_scriptFunctionPrefixes;

    QHash<QString, QJSValue> m_scriptWrappedFunctionCache;
    QJSValue m_byteArrayToScriptValueJSFunction;
    // Filesystem watcher for script auto-reload
    QFileSystemWatcher m_scriptWatcher;
    QFileInfo m_moduleFileInfo;
    QList<ControllerPreset::ScriptFileInfo> m_lastScriptFiles;

    bool m_bTesting;

    friend class ScriptConnection;
    friend class ControllerScriptInterface;
    friend class ColorJSProxy;
    friend class ColorMapperJSProxy;
    friend class ControllerEngineTest;
};
