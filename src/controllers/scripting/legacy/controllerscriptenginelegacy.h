#pragma once

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QMessageBox>

#include "controllers/legacycontrollermapping.h"
#include "controllers/scripting/controllerscriptenginebase.h"

/// ControllerScriptEngineLegacy loads and executes controller scripts for the legacy
/// JS/XML hybrid controller mapping system.
class ControllerScriptEngineLegacy : public ControllerScriptEngineBase {
    Q_OBJECT
  public:
    explicit ControllerScriptEngineLegacy(Controller* controller);
    ~ControllerScriptEngineLegacy() override;

    bool initialize() override;

    bool handleIncomingData(const QByteArray& data);

    /// Wrap a string of JS code in an anonymous function. This allows any JS
    /// string that evaluates to a function to be used in MIDI mapping XML files
    /// and ensures the function is executed with the correct 'this' object.
    QJSValue wrapFunctionCode(const QString& codeSnippet, int numberOfArgs);

  public slots:
    void setScriptFiles(const QList<LegacyControllerMapping::ScriptFileInfo>& scripts) {
        m_fileWatcher.removePaths(m_fileWatcher.files());
        m_scriptFiles = scripts;
    }

  protected:
    friend class ControllerTest;
    bool evaluateScriptFile(const QFileInfo& scriptFile);
    QJSValue evaluateCodeString(const QString& program,
            const QString& fileName = QString(),
            int lineNumber = 1);

  private:
    void shutdown() override;

    QJSValue wrapArrayBufferCallback(const QJSValue& callback);
    bool callFunctionOnObjects(const QList<QString>& scriptFunctionPrefixes,
            const QString&,
            const QJSValueList& args = QJSValueList(),
            bool bFatalError = false);

    QJSValue m_makeArrayBufferWrapperFunction;
    QList<QString> m_scriptFunctionPrefixes;
    QList<QJSValue> m_incomingDataFunctions;
    QHash<QString, QJSValue> m_scriptWrappedFunctionCache;
    QList<LegacyControllerMapping::ScriptFileInfo> m_scriptFiles;

    QFileSystemWatcher m_fileWatcher;

    // There is lots of tight coupling between ControllerScriptEngineLegacy
    // and ControllerScriptInterface. This is probably not worth improving in legacy code.
    friend class ControllerScriptInterfaceLegacy;
    std::shared_ptr<QJSEngine> jsEngine() const {
        return m_pJSEngine;
    }

    friend class ControllerScriptEngineLegacyTest;
};
