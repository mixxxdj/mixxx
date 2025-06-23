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
    explicit ControllerScriptEngineLegacy(
            Controller* controller, const RuntimeLoggingCategory& logger);
    ~ControllerScriptEngineLegacy() override;

    bool initialize() override;

    bool handleIncomingData(const QByteArray& data);

    /// Wrap a string of JS code in an anonymous function. This allows any JS
    /// string that evaluates to a function to be used in MIDI mapping XML files
    /// and ensures the function is executed with the correct 'this' object.
    QJSValue wrapFunctionCode(const QString& codeSnippet, int numberOfArgs);

    std::shared_ptr<QJSEngine> jsEngine() const {
        return m_pJSEngine;
    }

    void setScriptFiles(QList<LegacyControllerMapping::ScriptFileInfo> scripts);

    /// @brief Set the list of customizable settings and their currently set
    /// value, ready to be used. This method will generate a JSValue from their
    /// current state, meaning that any later mutation won't be used, and this
    /// method should be called again
    /// @param settings The list of settings in a valid state (initialized and
    /// restored)
    void setSettings(
            const QList<std::shared_ptr<AbstractLegacyControllerSetting>>& settings);

  private:
    struct Setting {
        QString name;
        QJSValue value;
    };

    bool evaluateScriptFile(const QFileInfo& scriptFile);
    void shutdown() override;

    QJSValue wrapArrayBufferCallback(const QJSValue& callback);
    bool callFunctionOnObjects(const QList<QString>& scriptFunctionPrefixes,
            const QString&,
            const QJSValueList& args = {},
            bool bFatalError = false);

    QJSValue m_makeArrayBufferWrapperFunction;
    QList<QString> m_scriptFunctionPrefixes;
    QList<QJSValue> m_incomingDataFunctions;
    QHash<QString, QJSValue> m_scriptWrappedFunctionCache;
    QList<LegacyControllerMapping::ScriptFileInfo> m_scriptFiles;
    QHash<QString, QJSValue> m_settings;

    QFileSystemWatcher m_fileWatcher;

    // There is lots of tight coupling between ControllerScriptEngineLegacy
    // and ControllerScriptInterface. This is probably not worth improving in legacy code.
    friend class ControllerScriptInterfaceLegacy;

    friend class ControllerScriptEngineLegacyTest;
    friend class MidiControllerTest;
};
