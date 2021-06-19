#include "controllers/scripting/controllerscriptenginebase.h"

#include "control/controlobject.h"
#include "controllers/controller.h"
#include "controllers/controllerdebug.h"
#include "controllers/scripting/colormapperjsproxy.h"
#include "errordialoghandler.h"
#include "mixer/playermanager.h"
#include "moc_controllerscriptenginebase.cpp"

ControllerScriptEngineBase::ControllerScriptEngineBase(Controller* controller)
        : m_bDisplayingExceptionDialog(false),
          m_pJSEngine(nullptr),
          m_pController(controller),
          m_bTesting(false) {
    // Handle error dialog buttons
    qRegisterMetaType<QMessageBox::StandardButton>("QMessageBox::StandardButton");
}

bool ControllerScriptEngineBase::initialize() {
    VERIFY_OR_DEBUG_ASSERT(!m_pJSEngine) {
        return false;
    }

    // Create the Script Engine
    m_pJSEngine = std::make_shared<QJSEngine>(this);

    QJSValue engineGlobalObject = m_pJSEngine->globalObject();

    QJSValue mapper = m_pJSEngine->newQMetaObject(
            &ColorMapperJSProxy::staticMetaObject);
    engineGlobalObject.setProperty("ColorMapper", mapper);

    if (m_pController) {
        qDebug() << "Controller in script engine is:"
                 << m_pController->getName();

        ControllerJSProxy* controllerProxy = m_pController->jsProxy();

        // Make the Controller instance available to scripts
        engineGlobalObject.setProperty(
                "controller", m_pJSEngine->newQObject(controllerProxy));

        // ...under the legacy name as well
        engineGlobalObject.setProperty(
                "midi", m_pJSEngine->newQObject(controllerProxy));
    }

    return true;
}

void ControllerScriptEngineBase::shutdown() {
    DEBUG_ASSERT(m_pJSEngine.use_count() == 1);
    m_pJSEngine.reset();
}

void ControllerScriptEngineBase::reload() {
    shutdown();
    initialize();
}

bool ControllerScriptEngineBase::executeFunction(
        QJSValue functionObject, const QJSValueList& args) {
    // This function is called from outside the controller engine, so we can't
    // use VERIFY_OR_DEBUG_ASSERT here
    if (!m_pJSEngine) {
        return false;
    }

    if (functionObject.isError()) {
        qDebug() << "ControllerScriptHandlerBase::executeFunction:"
                 << functionObject.toString();
        return false;
    }

    // If it's not a function, we're done.
    if (!functionObject.isCallable()) {
        qDebug() << "ControllerScriptHandlerBase::executeFunction:"
                 << functionObject.toVariant() << "Not a function";
        return false;
    }

    // If it does happen to be a function, call it.
    QJSValue returnValue = functionObject.call(args);
    if (returnValue.isError()) {
        showScriptExceptionDialog(returnValue);
        return false;
    }
    return true;
}

void ControllerScriptEngineBase::showScriptExceptionDialog(
        const QJSValue& evaluationResult, bool bFatalError) {
    VERIFY_OR_DEBUG_ASSERT(evaluationResult.isError()) {
        return;
    }

    QString errorMessage = evaluationResult.toString();
    QString line = evaluationResult.property("lineNumber").toString();
    QString backtrace = evaluationResult.property("stack").toString();
    QString filename = evaluationResult.property("fileName").toString();

    QString errorText;
    if (filename.isEmpty()) {
        errorText = QString("Uncaught exception at line %1 in passed code.")
                            .arg(line);
    } else {
        errorText = QString("Uncaught exception at line %1 in file %2.")
                            .arg(line, filename);
    }

    errorText += QStringLiteral("\n\nException:\n  ") + errorMessage;

    // Do not include backtrace in dialog key because it might contain midi
    // slider values that will differ most of the time. This would break
    // the "Ignore" feature of the error dialog.
    QString key = errorText;
    qWarning() << "ControllerScriptHandlerBase:" << errorText;

    // Add backtrace to the error details
    errorText += QStringLiteral("\n\nBacktrace:\n") + backtrace;

    if (!m_bDisplayingExceptionDialog) {
        scriptErrorDialog(errorText, key, bFatalError);
    }
}

void ControllerScriptEngineBase::scriptErrorDialog(
        const QString& detailedError, const QString& key, bool bFatalError) {
    if (m_bTesting) {
        return;
    }

    ErrorDialogProperties* props =
            ErrorDialogHandler::instance()->newDialogProperties();

    QString additionalErrorText;
    if (bFatalError) {
        additionalErrorText =
                tr("The functionality provided by this controller mapping will "
                   "be disabled until the issue has been resolved.");
    } else {
        // This happens when an exception is throws in an input handler (e. g.
        // when pressing a button on the midi controller).  In case you ignore
        // the issue, the button might not work if there's a bug in the
        // mapping, but the other buttons probably will.
        additionalErrorText =
                tr("You can ignore this error for this session but "
                   "you may experience erratic behavior.") +
                QString("<br>") +
                tr("Try to recover by resetting your controller.");
    }

    props->setType(DLG_WARNING);
    props->setTitle(tr("Controller Mapping Error"));
    props->setText(tr("The mapping for your controller \"%1\" is not working properly.")
                           .arg(m_pController->getName()));
    props->setInfoText(QStringLiteral("<html>") +
            tr("The script code needs to be fixed.") + QStringLiteral("<p>") +
            additionalErrorText + QStringLiteral("</p></html>"));

    // Add "Details" text and set monospace font since they may contain
    // backtraces and code.
    props->setDetails(detailedError, true);

    // To prevent multiple windows for the same error
    props->setKey(key);

    if (bFatalError) {
        props->addButton(QMessageBox::Close);
        props->setDefaultButton(QMessageBox::Close);
        props->setEscapeButton(QMessageBox::Close);
    } else {
        // Allow user to suppress further notifications about this particular
        // error
        props->addButton(QMessageBox::Ignore);
        props->addButton(QMessageBox::Retry);
        props->setDefaultButton(QMessageBox::Ignore);
        props->setEscapeButton(QMessageBox::Ignore);
    }
    props->setModal(false);

    if (ErrorDialogHandler::instance()->requestErrorDialog(props)) {
        m_bDisplayingExceptionDialog = true;
        // Enable custom handling of the dialog buttons
        connect(ErrorDialogHandler::instance(),
                &ErrorDialogHandler::stdButtonClicked,
                this,
                &ControllerScriptEngineBase::errorDialogButton);
    }
}

void ControllerScriptEngineBase::errorDialogButton(
        const QString& key, QMessageBox::StandardButton clickedButton) {
    Q_UNUSED(key);

    m_bDisplayingExceptionDialog = false;
    // Something was clicked, so disable this signal now
    disconnect(ErrorDialogHandler::instance(),
            &ErrorDialogHandler::stdButtonClicked,
            this,
            &ControllerScriptEngineBase::errorDialogButton);

    if (clickedButton == QMessageBox::Retry) {
        reload();
    }
}

void ControllerScriptEngineBase::throwJSError(const QString& message) {
    m_pJSEngine->throwError(message);
}
