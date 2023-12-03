#pragma once

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QMessageBox>
#ifdef MIXXX_USE_QML
#include <QQuickItem>
#endif

#include "controllers/legacycontrollermapping.h"
#include "controllers/scripting/controllerscriptenginebase.h"

#ifdef MIXXX_USE_QML
class ControllerRenderingEngine;
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

  public slots:
    void setScriptFiles(const QList<LegacyControllerMapping::ScriptFileInfo>& scripts);
#ifdef MIXXX_USE_QML
    void setLibraryDirectories(const QList<LegacyControllerMapping::QMLModuleInfo>& scripts);
    void setInfoScrens(const QList<LegacyControllerMapping::ScreenInfo>& scripts);

  private slots:
    void handleScreenFrame(
            const LegacyControllerMapping::ScreenInfo& screeninfo,
            const QImage& frame,
            const QDateTime& timestamp);

  signals:
    /// Emitted when a screen has been rendered
    // TODO (XXX) Move this signal in ControllerScriptEngineBase when ScreenInfo
    // isn't tight to LegacyControllerMapping anymore
    void previewRenderedScreen(const LegacyControllerMapping::ScreenInfo& screen, QImage frame);
#endif

  private:
    bool evaluateScriptFile(const QFileInfo& scriptFile);
#ifdef MIXXX_USE_QML
    bool bindSceneToScreen(
            const LegacyControllerMapping::ScriptFileInfo& qmlFile,
            const QString& screenIdentifier,
            std::shared_ptr<ControllerRenderingEngine> pScreen);
    void extractTranformFunction(const QMetaObject* metaObject, const QString& screenIdentifier);

    std::shared_ptr<QQuickItem> loadQMLFile(
            const LegacyControllerMapping::ScriptFileInfo& qmlScript,
            std::shared_ptr<ControllerRenderingEngine> pScreen);

    static QByteArray kScreenTranformFunctionUntypedSignature;
    static QByteArray kScreenTranformFunctionTypedSignature;

    struct TransformScreenFrameFunction {
        QMetaMethod method;
        bool typed;
    };
#endif

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
    QHash<QString, std::shared_ptr<QQuickItem>> m_rootItems;
    QHash<QString, TransformScreenFrameFunction> m_transformScreenFrameFunctions;
    QList<LegacyControllerMapping::QMLModuleInfo> m_libraryDirectories;
    QList<LegacyControllerMapping::ScreenInfo> m_infoScreens;
#endif
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
