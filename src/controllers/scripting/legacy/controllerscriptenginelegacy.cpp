#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"

#ifdef MIXXX_USE_QML
#include <QDirIterator>
#include <QtEndian>
#endif

#include "control/controlobject.h"
#include "controllers/controller.h"
#ifdef MIXXX_USE_QML
#include "controllers/rendering/controllerrenderingengine.h"
#endif
#include "controllers/scripting/colormapperjsproxy.h"
#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "errordialoghandler.h"
#include "mixer/playermanager.h"
#include "moc_controllerscriptenginelegacy.cpp"
#ifdef MIXXX_USE_QML
#include "util/assert.h"
#include "util/cmdlineargs.h"

QByteArray
        ControllerScriptEngineLegacy::kScreenTranformFunctionUntypedSignature =
                QMetaObject::normalizedSignature(
                        "transformFrame(QVariant,QVariant)");
QByteArray ControllerScriptEngineLegacy::kScreenTranformFunctionTypedSignature =
        QMetaObject::normalizedSignature("transformFrame(QVariant,QDateTime)");
#endif

ControllerScriptEngineLegacy::ControllerScriptEngineLegacy(
        Controller* controller, const RuntimeLoggingCategory& logger)
        : ControllerScriptEngineBase(controller, logger) {
    connect(&m_fileWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &ControllerScriptEngineLegacy::reload);
#ifdef MIXXX_USE_QML
    connect(&m_fileWatcher,
            &QFileSystemWatcher::directoryChanged,
            this,
            &ControllerScriptEngineLegacy::reload);
#endif
}

ControllerScriptEngineLegacy::~ControllerScriptEngineLegacy() {
    shutdown();
}

void ControllerScriptEngineLegacy::watchFilePath(const QString& path) {
    if (m_fileWatcher.files().contains(path) || m_fileWatcher.directories().contains(path)) {
        qDebug() << "File" << path << "is already being watch for controller auto-reload";
        return;
    }

    if (!m_fileWatcher.addPath(path)) {
        qCWarning(m_logger) << "Failed to watch script file"
                            << path;
    } else {
        qDebug() << "Watching file" << path << "for controller auto-reload";
    }
}

bool ControllerScriptEngineLegacy::callFunctionOnObjects(
        const QList<QString>& scriptFunctionPrefixes,
        const QString& function,
        const QJSValueList& args,
        bool bFatalError) {
    VERIFY_OR_DEBUG_ASSERT(m_pJSEngine) {
        return false;
    }

    const QJSValue global = m_pJSEngine->globalObject();

    bool success = true;
    for (const QString& prefixName : scriptFunctionPrefixes) {
        QJSValue prefix = global.property(prefixName);
        if (!prefix.isObject()) {
            qCWarning(m_logger) << "No" << prefixName << "object in script";
            continue;
        }

        QJSValue init = prefix.property(function);
        if (!init.isCallable()) {
            qCWarning(m_logger) << prefixName << "has no"
                                << function << " method";
            continue;
        }
        qCDebug(m_logger) << "Executing"
                          << prefixName << "." << function;
        QJSValue result = init.callWithInstance(prefix, args);
        if (result.isError()) {
            showScriptExceptionDialog(result, bFatalError);
            success = false;
        }
    }
#ifdef MIXXX_USE_QML
    if (m_bQmlMode) {
        QHashIterator<QString, std::shared_ptr<QQuickItem>> i(m_rootItems);
        while (i.hasNext()) {
            i.next();
            const QMetaObject* metaObject = i.value()->metaObject();
            QStringList argList;
            for (int i = 0; i < args.size(); i++)
                argList << "QVariant";
            int methodIdx =
                    metaObject->indexOfMethod(QString("%1(%2)")
                                                      .arg(function, argList.join(','))
                                                      .toUtf8());
            if (methodIdx == -1) {
                qCWarning(m_logger) << "QML Scene " << i.key() << "has no"
                                    << function << " method";
                continue;
            }
            QMetaMethod method = metaObject->method(methodIdx);
            qCDebug(m_logger) << "Executing"
                              << function << "on QML Scene " << i.key();

            VERIFY_OR_DEBUG_ASSERT(!m_pJSEngine->hasError()) {
                qWarning() << "Controller JS engine has an unhandled error. Discarding.";
                qDebug() << "Controller JS error is:" << m_pJSEngine->catchError().toString();
            }

            switch (args.size()) {
            case 0:
                success &= method.invoke(i.value().get(),
                        Qt::DirectConnection);
                break;
            case 1:
                success &= method.invoke(i.value().get(),
                        Qt::DirectConnection,
                        Q_ARG(QVariant, args[0].toVariant()));
                break;
            case 2:
                success &= method.invoke(i.value().get(),
                        Qt::DirectConnection,
                        Q_ARG(QVariant, args[0].toVariant()),
                        Q_ARG(QVariant, args[1].toVariant()));
                break;
            case 3:
                success &= method.invoke(i.value().get(),
                        Qt::DirectConnection,
                        Q_ARG(QVariant, args[0].toVariant()),
                        Q_ARG(QVariant, args[1].toVariant()),
                        Q_ARG(QVariant, args[2].toVariant()));
                break;
            case 4:
                success &= method.invoke(i.value().get(),
                        Qt::DirectConnection,
                        Q_ARG(QVariant, args[0].toVariant()),
                        Q_ARG(QVariant, args[1].toVariant()),
                        Q_ARG(QVariant, args[2].toVariant()),
                        Q_ARG(QVariant, args[3].toVariant()));
                break;
            default:
                qDebug() << "Trying to call a controller lifecycle method with "
                            "more than 5 args. Ignoring extra args";
            case 5:
                success &= method.invoke(i.value().get(),
                        Qt::DirectConnection,
                        Q_ARG(QVariant, args[0].toVariant()),
                        Q_ARG(QVariant, args[1].toVariant()),
                        Q_ARG(QVariant, args[2].toVariant()),
                        Q_ARG(QVariant, args[3].toVariant()),
                        Q_ARG(QVariant, args[4].toVariant()));
                break;
            }
        }
    }
#endif
    return success;
}

QJSValue ControllerScriptEngineLegacy::wrapFunctionCode(
        const QString& codeSnippet, int numberOfArgs) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (!m_pJSEngine) {
        return QJSValue();
    }

    QJSValue wrappedFunction;

    const auto it = m_scriptWrappedFunctionCache.constFind(codeSnippet);
    if (it != m_scriptWrappedFunctionCache.constEnd()) {
        wrappedFunction = it.value();
    } else {
        QStringList wrapperArgList;
        wrapperArgList.reserve(numberOfArgs);
        for (int i = 1; i <= numberOfArgs; i++) {
            wrapperArgList << QString("arg%1").arg(i);
        }
        QString wrapperArgs = wrapperArgList.join(",");
        QString wrappedCode = QStringLiteral("(function (") + wrapperArgs +
                QStringLiteral(") { (") + codeSnippet + QStringLiteral(")(") +
                wrapperArgs + QStringLiteral("); })");

        wrappedFunction = m_pJSEngine->evaluate(wrappedCode);
        if (wrappedFunction.isError()) {
            showScriptExceptionDialog(wrappedFunction);
        }
        m_scriptWrappedFunctionCache[codeSnippet] = wrappedFunction;
    }
    return wrappedFunction;
}

#ifdef MIXXX_USE_QML
void ControllerScriptEngineLegacy::setLibraryDirectories(
        const QList<LegacyControllerMapping::QMLModuleInfo>& directories) {
    const QStringList paths = m_fileWatcher.files();
    if (!paths.isEmpty()) {
        m_fileWatcher.removePaths(paths);
    }

    m_libraryDirectories = directories;
}
void ControllerScriptEngineLegacy::setInfoScrens(
        const QList<LegacyControllerMapping::ScreenInfo>& screens) {
    m_rootItems.clear();
    m_renderingScreens.clear();
    m_transformScreenFrameFunctions.clear();
    m_infoScreens = screens;
}
#endif

void ControllerScriptEngineLegacy::setScriptFiles(
        const QList<LegacyControllerMapping::ScriptFileInfo>& scripts) {
    const QStringList paths = m_fileWatcher.files();
    if (!paths.isEmpty()) {
        m_fileWatcher.removePaths(paths);
    }
    m_scriptFiles = scripts;

#ifdef MIXXX_USE_QML
    for (const LegacyControllerMapping::ScriptFileInfo& script : std::as_const(m_scriptFiles)) {
        if (script.type == LegacyControllerMapping::ScriptFileInfo::Type::QML) {
            setQMLMode(true);
            return;
        }
    }

    setQMLMode(false);
#endif
}

bool ControllerScriptEngineLegacy::initialize() {
    if (!ControllerScriptEngineBase::initialize()) {
        return false;
    }

#ifdef MIXXX_USE_QML
    // During the initialisation, any QML errors are considered fatal
    setErrorsAreFatal(true);
    QMap<QString, std::shared_ptr<ControllerRenderingEngine>> availableScreens;

    if (m_bQmlMode) {
        for (const LegacyControllerMapping::ScreenInfo& screen : std::as_const(m_infoScreens)) {
            VERIFY_OR_DEBUG_ASSERT(!availableScreens.contains(screen.identifier)) {
                qWarning() << "A controller screen already contains the "
                              "identifier "
                           << screen.identifier;
                return false;
            }
            availableScreens.insert(screen.identifier,
                    std::make_shared<ControllerRenderingEngine>(screen, this));

            if (!availableScreens.value(screen.identifier)->isValid()) {
                qWarning() << QString(
                        "Unable to start the screen render for %1.")
                                      .arg(screen.identifier);
                return false;
            }

            if (m_bTesting)
                continue;

            availableScreens.value(screen.identifier)
                    ->requestSetup(
                            std::dynamic_pointer_cast<QQmlEngine>(m_pJSEngine));

            if (!availableScreens.value(screen.identifier)->isValid()) {
                qWarning() << QString(
                        "Unable to setup the screen render for %1.")
                                      .arg(screen.identifier);
                availableScreens.value(screen.identifier)->stop();
                return false;
            }
        }
    } else if (!m_infoScreens.isEmpty()) {
        qWarning() << "Controller mapping has screen definitions but no QML "
                      "files to render on it. Ignoring.";
    }
#endif

    // Binary data is passed from the Controller as a QByteArray, which
    // QJSEngine::toScriptValue converts to an ArrayBuffer in JavaScript.
    // ArrayBuffer cannot be accessed with the [] operator in JS; it needs
    // to be converted to a typed array (Uint8Array in this case) first.
    // This function generates a wrapper function from a JS callback to do
    // that conversion automatically.
    m_makeArrayBufferWrapperFunction = m_pJSEngine->evaluate(QStringLiteral(
            // arg2 is the timestamp for ControllerScriptModuleEngine.
            // In ControllerScriptEngineLegacy it is the length of the array.
            "(function(callback) {"
            "    return function(arrayBuffer, arg2) {"
            "        callback(new Uint8Array(arrayBuffer), arg2);"
            "    };"
            "})"));

    // Make this ControllerScriptHandler instance available to scripts as 'engine'.
    QJSValue engineGlobalObject = m_pJSEngine->globalObject();
    ControllerScriptInterfaceLegacy* legacyScriptInterface =
            new ControllerScriptInterfaceLegacy(this, m_logger);
    engineGlobalObject.setProperty(
            "engine", m_pJSEngine->newQObject(legacyScriptInterface));

#ifdef MIXXX_USE_QML
    if (m_bQmlMode) {
        for (const LegacyControllerMapping::QMLModuleInfo& module :
                std::as_const(m_libraryDirectories)) {
            auto path = module.dirinfo.absoluteFilePath();
            QDirIterator it(path,
                    QStringList() << "*.qml",
                    QDir::Files,
                    QDirIterator::Subdirectories);
            while (it.hasNext()) {
                watchFilePath(it.next());
            }
            watchFilePath(path);
            std::dynamic_pointer_cast<QQmlEngine>(m_pJSEngine)->addImportPath(path);
        }
    } else if (!m_libraryDirectories.isEmpty()) {
        qWarning() << "Controller mapping has QML library definitions but no "
                      "QML files to use it. Ignoring.";
    }

    // If we encounter a failure while loading a scene, we will need to properly
    // stop the screen threads before shutting down.
    bool sceneBindingHasFailure = false;
#endif
    for (const LegacyControllerMapping::ScriptFileInfo& script : std::as_const(m_scriptFiles)) {
#ifdef MIXXX_USE_QML
        if (script.type == LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT) {
#endif
            if (!evaluateScriptFile(script.file)) {
                shutdown();
                return false;
            }
            if (!script.identifier.isEmpty()) {
                m_scriptFunctionPrefixes.append(script.identifier);
            }
#ifdef MIXXX_USE_QML
        } else {
            if (script.identifier.isEmpty()) {
                while (!availableScreens.isEmpty()) {
                    QString screenIdentifier(availableScreens.firstKey());
                    if (!bindSceneToScreen(script,
                                screenIdentifier,
                                availableScreens.take(screenIdentifier))) {
                        sceneBindingHasFailure = true;
                    }
                }
            } else {
                if (!availableScreens.contains(script.identifier)) {
                    qCritical() << "Not screen" << script.identifier << "found!";

                    sceneBindingHasFailure = true;
                    break;
                }
                if (!bindSceneToScreen(script,
                            script.identifier,
                            availableScreens.take(script.identifier))) {
                    sceneBindingHasFailure = true;
                }
            }
        }
    }

    if (!availableScreens.isEmpty()) {
        if (!sceneBindingHasFailure)
            qWarning()
                    << "Found screen with no QML scene able to run on it. Ignoring"
                    << availableScreens.size() << "screens";

        while (!availableScreens.isEmpty()) {
            auto orphanScreen = availableScreens.take(availableScreens.firstKey());
            std::move(orphanScreen)->deleteLater();
        }
    }
    if (sceneBindingHasFailure) {
        shutdown();
        return false;
#endif
    }

    // For testing, do not actually initialize the scripts, just check for
    // syntax errors above.
    if (m_bTesting) {
        return true;
    }

    for (QString functionName : std::as_const(m_scriptFunctionPrefixes)) {
        if (functionName.isEmpty()) {
            continue;
        }
        functionName.append(QStringLiteral(".incomingData"));
        m_incomingDataFunctions.append(
                wrapArrayBufferCallback(
                        wrapFunctionCode(functionName, 2)));
    }

    // m_pController is nullptr in tests.
    const auto controllerName = m_pController ? m_pController->getName() : QString{};
    const auto args = QJSValueList{
            controllerName,
            m_logger().isDebugEnabled(),
    };

#ifdef MIXXX_USE_QML
    setCanPause(true);
    for (const auto& pScreen : qAsConst(m_renderingScreens)) {
        pScreen->start();
    }
#endif

    if (
            !callFunctionOnObjects(m_scriptFunctionPrefixes, "init", args, true)) {
        shutdown();
        return false;
    }
#ifdef MIXXX_USE_QML
    // At runtime, QML errors aren't considered fatal anymore now that the engine has started
    setErrorsAreFatal(false);
#endif

    return true;
}

#ifdef MIXXX_USE_QML
void ControllerScriptEngineLegacy::extractTranformFunction(
        const QMetaObject* metaObject, const QString& screenIdentifier) {
    VERIFY_OR_DEBUG_ASSERT(metaObject) {
        qWarning() << "Invalid meta object for screen" << screenIdentifier
                   << "It may be that an unhandled issue occurred when imnporting the scene.";
        return;
    }

    QMetaMethod tranformFunction;
    bool typed = false;
    int methodIdx = metaObject->indexOfMethod(kScreenTranformFunctionUntypedSignature);

    if (methodIdx == -1 || !metaObject->method(methodIdx).isValid()) {
        qDebug() << "QML Scene for screen" << screenIdentifier
                 << "has no valid untyped transformFrame method.";
        methodIdx = metaObject->indexOfMethod(kScreenTranformFunctionTypedSignature);
        typed = true;
    }

    tranformFunction = metaObject->method(methodIdx);

    if (!tranformFunction.isValid()) {
        qDebug() << "QML Scene for screen" << screenIdentifier
                 << "has no valid typed transformFrame method. The frame data will be sent "
                    "untransformed";
        QStringList methods;
        for (int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i)
            methods << QString::fromLatin1(metaObject->method(i).methodSignature());
        qDebug() << "Found methods are: " << methods.join(", ");
    }

    m_transformScreenFrameFunctions.insert(screenIdentifier,
            TransformScreenFrameFunction{tranformFunction, typed});
}

bool ControllerScriptEngineLegacy::bindSceneToScreen(
        const LegacyControllerMapping::ScriptFileInfo& qmlFile,
        const QString& screenIdentifier,
        std::shared_ptr<ControllerRenderingEngine> pScreen) {
    // Like for Javascript, if the script is invalid, it should be watched so the user can fix it
    // without having to restart Mixxx. So, add it to the watcher before
    // evaluating it.
    watchFilePath(qmlFile.file.absoluteFilePath());

    auto pScene = loadQMLFile(qmlFile, pScreen);
    if (!pScene) {
        pScreen->stop();
        std::move(pScreen)->deleteLater();
        return false;
    }
    const QMetaObject* metaObject = pScene->metaObject();

    extractTranformFunction(metaObject, screenIdentifier);

    connect(pScreen.get(),
            &ControllerRenderingEngine::frameRendered,
            this,
            &ControllerScriptEngineLegacy::handleScreenFrame);
    m_renderingScreens.insert(screenIdentifier, pScreen);
    m_rootItems.insert(screenIdentifier, pScene);
    return true;
}

void ControllerScriptEngineLegacy::handleScreenFrame(
        const LegacyControllerMapping::ScreenInfo& screeninfo,
        const QImage& frame,
        const QDateTime& timestamp) {
    VERIFY_OR_DEBUG_ASSERT(
            m_transformScreenFrameFunctions.contains(screeninfo.identifier) ||
            m_renderingScreens.contains(screeninfo.identifier)) {
        qWarning() << "Unable to find transform function info for the given screen";
        return;
    };
    VERIFY_OR_DEBUG_ASSERT(m_rootItems.contains(screeninfo.identifier)) {
        qWarning() << "Unable to find a root item for the given screen";
        return;
    };

    if (CmdlineArgs::Instance().getControllerPreviewScreens()) {
        QImage screenDebug(frame);

        if (screeninfo.endian != std::endian::native) {
            switch (screeninfo.endian) {
            case std::endian::big:
                qFromBigEndian<ushort>(frame.constBits(),
                        frame.sizeInBytes() / 2,
                        screenDebug.bits());
                break;
            case std::endian::little:
                qFromLittleEndian<ushort>(frame.constBits(),
                        frame.sizeInBytes() / 2,
                        screenDebug.bits());
                break;
            default:
                break;
            }
        }
        if (screeninfo.reversedColor) {
            screenDebug.rgbSwap();
        }

        emit previewRenderedScreen(screeninfo, screenDebug);
    }

    QByteArray input((const char*)frame.constBits(), frame.sizeInBytes());
    const TransformScreenFrameFunction& tranformMethod =
            m_transformScreenFrameFunctions[screeninfo.identifier];

    if (!tranformMethod.method.isValid() && screeninfo.rawData) {
        m_renderingScreens[screeninfo.identifier]->requestSend(m_pController, input);
        return;
    }

    if (!tranformMethod.method.isValid()) {
        qWarning()
                << "Could not find a valid transform function but the screen "
                   "doesn't accept raw data. Aborting screen rendering.";
        m_renderingScreens[screeninfo.identifier]->stop();
        return;
    }

    QVariant returnedValue;

    VERIFY_OR_DEBUG_ASSERT(!m_pJSEngine->hasError()) {
        qWarning() << "Controller JS engine has an unhandled error. Discarding.";
        qDebug() << "Controller JS error is:" << m_pJSEngine->catchError().toString();
    }
    // During the frame transformation, any QML errors are considered fatal
    setErrorsAreFatal(true);
    bool isSuccessful = tranformMethod.typed
            ? tranformMethod.method.invoke(
                      m_rootItems.value(screeninfo.identifier).get(),
                      Qt::DirectConnection,
                      Q_RETURN_ARG(QVariant, returnedValue),
                      Q_ARG(QVariant, input),
                      Q_ARG(QDateTime, timestamp))
            : tranformMethod.method.invoke(
                      m_rootItems.value(screeninfo.identifier).get(),
                      Qt::DirectConnection,
                      Q_RETURN_ARG(QVariant, returnedValue),
                      Q_ARG(QVariant, input),
                      Q_ARG(QVariant, timestamp));
    setErrorsAreFatal(false);

    if (!isSuccessful) {
        qWarning() << "Could not transform rendering buffer for sreeen" << screeninfo.identifier;

        // We manually stop the screen before we trigger the shutdown procedure
        // as this last one may continue rendering process in order to perform
        // screen splash off
        m_renderingScreens[screeninfo.identifier]->stop();
        shutdown();
        return;
    }
    if (!isSuccessful || !returnedValue.isValid()) {
        qWarning() << "Could not transform rendering buffer. The transform "
                      "function didn't return the expected Array. Stopping "
                      "rendering on this screen";
        m_renderingScreens[screeninfo.identifier]->stop();
        return;
    }

    QByteArray transformedFrame;

    if (returnedValue.canView<QByteArray>()) {
        transformedFrame = returnedValue.view<QByteArray>();
    } else if (returnedValue.canConvert<QByteArray>()) {
        transformedFrame = returnedValue.toByteArray();
    } else {
        qWarning() << "Unable to interpret the returned data " << returnedValue;
        return;
    }

    if (CmdlineArgs::Instance().getControllerDebug()) {
        qDebug() << "Transform screen data for screen " << screeninfo.identifier
                 << "(first 64 bytes)"
                 << QByteArray(transformedFrame.toHex(' '), 128);
        m_pController->sendBytes(returnedValue.view<QByteArray>());
    }

    m_renderingScreens[screeninfo.identifier]->requestSend(
            m_pController, transformedFrame);
}
#endif

void ControllerScriptEngineLegacy::shutdown() {
    // There is no js engine if the mapping was not loaded from a file but by
    // creating a new, empty mapping LegacyMidiControllerMapping with the wizard
    if (m_pJSEngine) {
        callFunctionOnObjects(m_scriptFunctionPrefixes, "shutdown");
    }

#ifdef MIXXX_USE_QML
    setCanPause(false);
    // Wait till the splash off animation has finished rendering
    uint maxSplashOffDuration = 0;
    for (const auto& pScreen : qAsConst(m_renderingScreens)) {
        if (!pScreen->isRunning())
            continue;
        maxSplashOffDuration = qMax(maxSplashOffDuration, pScreen->info().splash_off);
    }

    auto splashOffDeadline = mixxx::Duration::fromMillis(maxSplashOffDuration) +
            mixxx::Time::elapsed();
    while (splashOffDeadline > mixxx::Time::elapsed()) {
        QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents,
                (splashOffDeadline - mixxx::Time::elapsed()).toIntegerMillis());
    }

    m_rootItems.clear();
    for (const auto& pScreen : qAsConst(m_renderingScreens)) {
        VERIFY_OR_DEBUG_ASSERT(!pScreen->isValid() ||
                !pScreen->isRunning() || pScreen->stop()) {
            qWarning() << "Unable to stop the screen";
        };
    }
    m_renderingScreens.clear();
    m_transformScreenFrameFunctions.clear();
#endif
    m_scriptWrappedFunctionCache.clear();
    m_incomingDataFunctions.clear();
    m_scriptFunctionPrefixes.clear();
    if (m_pJSEngine) {
        ControllerScriptEngineBase::shutdown();
    }
}

bool ControllerScriptEngineLegacy::handleIncomingData(const QByteArray& data) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (!m_pJSEngine) {
        return false;
    }

    const auto args = QJSValueList{
            m_pJSEngine->toScriptValue(data),
            static_cast<uint>(data.size()),
    };

    for (auto&& function : m_incomingDataFunctions) {
        ControllerScriptEngineBase::executeFunction(&function, args);
    }

    return true;
}

bool ControllerScriptEngineLegacy::evaluateScriptFile(const QFileInfo& scriptFile) {
    VERIFY_OR_DEBUG_ASSERT(m_pJSEngine) {
        return false;
    }

    if (!scriptFile.exists()) {
        qCWarning(m_logger) << "File does not exist:"
                            << scriptFile.absoluteFilePath();
        return false;
    }

    // If the script is invalid, it should be watched so the user can fix it
    // without having to restart Mixxx. So, add it to the watcher before
    // evaluating it.
    watchFilePath(scriptFile.absoluteFilePath());
    qCDebug(m_logger) << "Loading"
                      << scriptFile.absoluteFilePath();

    // Read in the script file
    QString filename = scriptFile.absoluteFilePath();
    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly)) {
        qCWarning(m_logger) << QString(
                "Problem opening the script file: %1, "
                "error # %2, %3")
                                       .arg(filename,
                                               QString::number(input.error()),
                                               input.errorString());
        // Set up error dialog
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(tr("Controller Mapping File Problem"));
        props->setText(tr("The mapping for controller \"%1\" cannot be opened.")
                               .arg(m_pController->getName()));
        props->setInfoText(
                tr("The functionality provided by this controller mapping will "
                   "be disabled until the issue has been resolved."));

        // We usually don't translate the details field, but the cause of
        // this problem lies in the user's system (e.g. a permission
        // issue). Translating this will help users to fix the issue even
        // when they don't speak english.
        props->setDetails(tr("File:") + QStringLiteral(" ") + filename +
                QStringLiteral("\n") + tr("Error:") + QStringLiteral(" ") +
                input.errorString());

        // Ask above layer to display the dialog & handle user response
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return false;
    }

    QString scriptCode = QString(input.readAll()) + QStringLiteral("\n");
    input.close();

    QJSValue scriptFunction = m_pJSEngine->evaluate(scriptCode, filename);
    if (scriptFunction.isError()) {
        showScriptExceptionDialog(scriptFunction, true);
        return false;
    }

    return true;
}

#ifdef MIXXX_USE_QML
std::shared_ptr<QQuickItem> ControllerScriptEngineLegacy::loadQMLFile(
        const LegacyControllerMapping::ScriptFileInfo& qmlScript,
        std::shared_ptr<ControllerRenderingEngine> pScreen) {
    VERIFY_OR_DEBUG_ASSERT(m_pJSEngine ||
            qmlScript.type !=
                    LegacyControllerMapping::ScriptFileInfo::Type::QML) {
        return std::shared_ptr<QQuickItem>(nullptr);
    }

    std::unique_ptr<QQmlComponent> qmlComponent =
            std::make_unique<QQmlComponent>(
                    std::dynamic_pointer_cast<QQmlEngine>(m_pJSEngine).get(),
                    QUrl::fromLocalFile(qmlScript.file.absoluteFilePath()),
                    QQmlComponent::PreferSynchronous);
    while (qmlComponent->isLoading()) {
        qDebug() << "Waiting for component "
                 << qmlScript.file.absoluteFilePath()
                 << " to be ready: " << qmlComponent->progress();
        QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 500);
    }

    if (qmlComponent->isError()) {
        const QList<QQmlError> errorList = qmlComponent->errors();
        for (const QQmlError& error : errorList) {
            qWarning() << "Unable to load the QML scene:" << error.url()
                       << "at line" << error.line() << ", error: " << error;
            showQMLExceptionDialog(error, true);
        }
        return std::shared_ptr<QQuickItem>(nullptr);
    }

    VERIFY_OR_DEBUG_ASSERT(qmlComponent->isReady()) {
        qWarning() << "QMLComponent isn't ready although synchronous load was requested.";
        return std::shared_ptr<QQuickItem>(nullptr);
    }

    QObject* pRootObject = qmlComponent->createWithInitialProperties(
            QVariantMap{{"screenId", pScreen->info().identifier}});
    if (qmlComponent->isError()) {
        const QList<QQmlError> errorList = qmlComponent->errors();
        for (const QQmlError& error : errorList)
            qWarning() << error.url() << error.line() << error;
        return std::shared_ptr<QQuickItem>(nullptr);
    }

    std::shared_ptr<QQuickItem> rootItem =
            std::shared_ptr<QQuickItem>(qobject_cast<QQuickItem*>(pRootObject));
    if (!rootItem) {
        qWarning("run: Not a QQuickItem");
        delete pRootObject;
        return std::shared_ptr<QQuickItem>(nullptr);
    }

    watchFilePath(qmlScript.file.absoluteFilePath());

    // The root item is ready. Associate it with the window.
    if (!m_bTesting) {
        rootItem->setParentItem(pScreen->quickWindow()->contentItem());

        rootItem->setWidth(pScreen->quickWindow()->width());
        rootItem->setHeight(pScreen->quickWindow()->height());
    }

    return rootItem;
}
#endif

QJSValue ControllerScriptEngineLegacy::wrapArrayBufferCallback(const QJSValue& callback) {
    return m_makeArrayBufferWrapperFunction.call(QJSValueList{callback});
}
