#pragma once

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QMessageBox>
#include <memory>
#ifdef MIXXX_USE_QML
#include <QMetaMethod>
#include <unordered_map>
#endif

#include "controllers/legacycontrollermapping.h"
#include "controllers/scripting/controllerscriptenginebase.h"

#ifdef MIXXX_USE_QML
class QQuickItem;
class ControllerRenderingEngine;
namespace mixxx {
namespace qml {
class QmlMixxxControllerScreen;
} // namespace qml
} // namespace mixxx
#endif

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

#ifdef MIXXX_USE_QML
    void setModulePaths(const QList<LegacyControllerMapping::QMLModuleInfo>& scripts);
    void setInfoScreens(const QList<LegacyControllerMapping::ScreenInfo>& scripts);
    void setResourcePath(const QString& resourcePath) {
        m_resourcePath = resourcePath;
    }

  private slots:
    void handleScreenFrame(
            const LegacyControllerMapping::ScreenInfo& screeninfo,
            const QImage& frame,
            const QDateTime& timestamp);

  signals:
    /// Emitted when a screen has been rendered.
    // TODO (XXX) Move this signal in ControllerScriptEngineBase when ScreenInfo
    // isn't tight to LegacyControllerMapping anymore.
    void previewRenderedScreen(const LegacyControllerMapping::ScreenInfo& screen, QImage frame);
#endif

  private:
    struct Setting {
        QString name;
        QJSValue value;
    };

    bool evaluateScriptFile(const QFileInfo& scriptFile);
#ifdef MIXXX_USE_QML
    bool bindSceneToScreen(
            const LegacyControllerMapping::ScriptFileInfo& qmlFile,
            const QString& screenIdentifier,
            std::shared_ptr<ControllerRenderingEngine> pScreen);
    void extractTransformFunction(const QMetaObject* metaObject, const QString& screenIdentifier);

    std::unique_ptr<mixxx::qml::QmlMixxxControllerScreen> loadQMLFile(
            const LegacyControllerMapping::ScriptFileInfo& qmlScript,
            std::shared_ptr<ControllerRenderingEngine> pScreen);

    struct TransformScreenFrameFunction {
        QMetaMethod method;
        bool typed;
    };
#endif

    /// @brief Call the shutdown hook on the mapping script.
    /// @return true if the hook was run successfully, or if there was none.
    bool callShutdownFunction();
    /// @brief Call the init hook on the mapping script.
    /// @return true if the hook was run successfully, or if there was none.
    bool callInitFunction();
    void shutdown() override;
    QJSValue wrapArrayBufferCallback(const QJSValue& callback);
    bool callFunctionOnObjects(const QList<QString>& scriptFunctionPrefixes,
            const QString&,
            const QJSValueList& args = {},
            bool bFatalError = false);
    void watchFilePath(const QString& path);

    QJSValue m_makeArrayBufferWrapperFunction;
    QList<QString> m_scriptFunctionPrefixes;
#ifdef MIXXX_USE_QML
    QHash<QString, std::shared_ptr<ControllerRenderingEngine>> m_renderingScreens;
    // Contains all the scenes loaded for this mapping. Key is the scene
    // identifier (LegacyControllerMapping::ScreenInfo::identifier), value in
    // the QML root item.
    std::unordered_map<QString, std::unique_ptr<mixxx::qml::QmlMixxxControllerScreen>> m_rootItems;
    QList<LegacyControllerMapping::QMLModuleInfo> m_modules;
    QList<LegacyControllerMapping::ScreenInfo> m_infoScreens;
    QString m_resourcePath;
#endif
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
