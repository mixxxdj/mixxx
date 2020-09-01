#pragma once

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QMessageBox>

#include "controllers/controllerpreset.h"
#include "controllers/scripting/controllerscriptenginebase.h"
#include "util/duration.h"

class Controller;
class ControllerScriptInterfaceLegacy;
class EvaluationException;
class ScriptConnection;

/// ControllerScriptEngineLegacy loads and executes controller scripts for the legacy
/// JS/XML hybrid controller mapping system.
class ControllerScriptEngineLegacy : public ControllerScriptEngineBase {
    Q_OBJECT
  public:
    ControllerScriptEngineLegacy(Controller* controller);

    bool initialize() override;

    bool handleIncomingData(const QByteArray& data);

    /// Wrap a string of JS code in an anonymous function. This allows any JS
    /// string that evaluates to a function to be used in MIDI mapping XML files
    /// and ensures the function is executed with the correct 'this' object.
    QJSValue wrapFunctionCode(const QString& codeSnippet, int numberOfArgs);

    // There is lots of tight coupling between ControllerScriptEngineLegacy
    // and ControllerScriptInterface. This is probably not worth improving in legacy code.
    QJSEngine* jsEngine() {
        return m_pJSEngine;
    }

  public slots:
    void setScriptFiles(const QList<ControllerPreset::ScriptFileInfo>& scripts) {
        m_scriptFiles = scripts;
    }

  private:
    bool evaluateScriptFile(const QFileInfo& scriptFile);
    void shutdown() override;

    void generateScriptFunctions(const QString& code);

    bool callFunctionOnObjects(QList<QString>,
            const QString&,
            QJSValueList args = QJSValueList(),
            bool bFatalError = false);

    QList<QString> m_scriptFunctionPrefixes;
    QList<QJSValue> m_incomingDataFunctions;
    QHash<QString, QJSValue> m_scriptWrappedFunctionCache;
    QList<ControllerPreset::ScriptFileInfo> m_scriptFiles;

    friend class ScriptConnection;
    friend class ControllerScriptInterfaceLegacy;
    friend class ColorJSProxy;
    friend class ColorMapperJSProxy;
    friend class ControllerEngineTest;
};
