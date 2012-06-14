/***************************************************************************
                          controllerengine.cpp  -  description
                          -------------------
    begin                : Sat Apr 30 2011
    copyright            : (C) 2011 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

#include "controllers/controllerengine.h"

#include "controllers/controller.h"
#include "controllers/defs_controllers.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "errordialoghandler.h"

// #include <QScriptSyntaxCheckResult>

// Used for id's inside controlConnection objects
// (closure compatible version of connectControl)
#include <QUuid>

#ifdef _MSC_VER
    #include <float.h>  // for _isnan() on VC++
    #define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#else
    #include <math.h>  // for isnan() everywhere else
#endif

const int kDecks = 16;

ControllerEngine::ControllerEngine(Controller* controller)
    : m_pEngine(NULL),
      m_pController(controller),
      m_bDebug(false),
      m_bPopups(false),
      m_pBaClass(NULL) {

    // Handle error dialog buttons
    qRegisterMetaType<QMessageBox::StandardButton>("QMessageBox::StandardButton");

    // Pre-allocate arrays for average number of virtual decks
    m_intervalAccumulator.resize(kDecks);
    m_lastMovement.resize(kDecks);
    m_dx.resize(kDecks);
    m_rampTo.resize(kDecks);
    m_ramp.resize(kDecks);
    m_pitchFilter.resize(kDecks);
    m_rampFactor.resize(kDecks);
    m_brakeActive.resize(kDecks);
    m_brakeKeylock.resize(kDecks);

    // Initialize arrays used for testing and pointers
    for (int i=0; i < kDecks; i++) {
        m_dx[i] = 0.0;
        m_pitchFilter[i] = new PitchFilter();
        m_ramp[i] = false;
    }

    initializeScriptEngine();
}

ControllerEngine::~ControllerEngine() {
    // Clean up
    for (int i=0; i < kDecks; i++) {
        delete m_pitchFilter[i];
        m_pitchFilter[i] = NULL;
    }

    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if (m_pEngine != NULL) {
        QScriptEngine *engine = m_pEngine;
        m_pEngine = NULL;
        engine->deleteLater();
    }

}

/* -------- ------------------------------------------------------
Purpose: Calls the same method on a list of JS Objects
Input:   -
Output:  -
-------- ------------------------------------------------------ */
void ControllerEngine::callFunctionOnObjects(QList<QString> scriptFunctionPrefixes,
                                             QString function, QScriptValueList args) {
    const QScriptValue global = m_pEngine->globalObject();

    foreach (QString prefixName, scriptFunctionPrefixes) {
        QScriptValue prefix = global.property(prefixName);
        if (!prefix.isValid() || !prefix.isObject()) {
            qWarning() << "ControllerEngine: No" << prefixName << "object in script";
            continue;
        }

        QScriptValue init = prefix.property(function);
        if (!init.isValid() || !init.isFunction()) {
            qWarning() << "ControllerEngine:" << prefixName << "has no" << function << " method";
            continue;
        }
        if (m_bDebug) {
            qDebug() << "ControllerEngine: Executing" << prefixName << "." << function;
        }
        init.call(prefix, args);
    }
}

/* -------- ------------------------------------------------------
Purpose: Resolves a function name to a QScriptValue including
            OBJECT.Function calls
Input:   -
Output:  -
-------- ------------------------------------------------------ */
QScriptValue ControllerEngine::resolveFunction(QString function, bool useCache) const {
    if (useCache && m_scriptValueCache.contains(function)) {
        return m_scriptValueCache.value(function);
    }

    QScriptValue object = m_pEngine->globalObject();
    QStringList parts = function.split(".");

    for (int i = 0; i < parts.size(); i++) {
        object = object.property(parts.at(i));
        if (!object.isValid())
            return QScriptValue();
    }

    if (!object.isFunction()) {
        return QScriptValue();
    }
    m_scriptValueCache[function] = object;
    return object;
}

/* -------- ------------------------------------------------------
Purpose: Shuts down scripts in an orderly fashion
            (stops timers then executes shutdown functions)
Input:   -
Output:  -
-------- ------------------------------------------------------ */
void ControllerEngine::gracefulShutdown() {
    qDebug() << "ControllerEngine shutting down...";

    // Clear the m_connectedControls hash so we stop responding
    // to signals.
    m_connectedControls.clear();

    // Stop all timers
    stopAllTimers();

    // Call each script's shutdown function if it exists
    callFunctionOnObjects(m_scriptFunctionPrefixes, "shutdown");

    // Prevents leaving decks in an unstable state
    //  if the controller is shut down while scratching
    QHashIterator<int, int> i(m_scratchTimers);
    while (i.hasNext()) {
        i.next();
        qDebug() << "Aborting scratching on deck" << i.value();
        // Clear scratch2_enable
        QString group = QString("[Channel%1]").arg(i.value());
        ControlObjectThread *cot = getControlObjectThread(group, "scratch2_enable");
        if (cot != NULL) {
            cot->slotSet(0);
        }
    }

    // Clear the Script Value cache
    m_scriptValueCache.clear();

    // Free all the control object threads
    QList<ConfigKey> keys = m_controlCache.keys();
    QList<ConfigKey>::iterator it = keys.begin();
    QList<ConfigKey>::iterator end = keys.end();
    while (it != end) {
        ConfigKey key = *it;
        ControlObjectThread *cot = m_controlCache.take(key);
        delete cot;
        it++;
    }

    delete m_pBaClass;
    m_pBaClass = NULL;
}

bool ControllerEngine::isReady() {
    bool ret = m_pEngine != NULL;
    return ret;
}

void ControllerEngine::initializeScriptEngine() {
    // Create the Script Engine
    m_pEngine = new QScriptEngine(this);

    // Make this ControllerEngine instance available to scripts as 'engine'.
    QScriptValue engineGlobalObject = m_pEngine->globalObject();
    engineGlobalObject.setProperty("engine", m_pEngine->newQObject(this));

    if (m_pController) {
        qDebug() << "Controller in script engine is:" << m_pController->getName();

        // Make the Controller instance available to scripts
        engineGlobalObject.setProperty("controller", m_pEngine->newQObject(m_pController));

        // ...under the legacy name as well
        engineGlobalObject.setProperty("midi", m_pEngine->newQObject(m_pController));
    }

    m_pBaClass = new ByteArrayClass(m_pEngine);
    engineGlobalObject.setProperty("ByteArray", m_pBaClass->constructor());
}

/* -------- ------------------------------------------------------
   Purpose: Load all script files given in the supplied list
   Input:   Global ConfigObject, QString list of file names to load
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::loadScriptFiles(QString configPath,
                                       QList<QString> scriptFileNames) {
    // Set the Debug flag
    if (m_pController)
        m_bDebug = m_pController->debugging();

    qDebug() << "ControllerEngine: Loading & evaluating all script code";

    m_lastConfigPath = configPath;

    // scriptPaths holds the paths to search in when we're looking for scripts
    QList<QString> scriptPaths;
    scriptPaths.append(USER_PRESETS_PATH);
    scriptPaths.append(LOCAL_PRESETS_PATH);
    scriptPaths.append(configPath.append("controllers/"));

    foreach (QString curScriptFileName, scriptFileNames) {
        evaluate(curScriptFileName, scriptPaths);

        if (m_scriptErrors.contains(curScriptFileName)) {
            qDebug() << "Errors occured while loading " << curScriptFileName;
        }
    }

    connect(&m_scriptWatcher, SIGNAL(fileChanged(QString)),
            this, SLOT(scriptHasChanged(QString)));

    emit(initialized());
}

// Slot to run when a script file has changed
void ControllerEngine::scriptHasChanged(QString scriptFilename) {
    qDebug() << "ControllerEngine: Reloading Scripts";
    ControllerPresetPointer pPreset = m_pController->getPreset();

    disconnect(&m_scriptWatcher, SIGNAL(fileChanged(QString)),
               this, SLOT(scriptHasChanged(QString)));

    gracefulShutdown();

    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if (m_pEngine != NULL) {
        QScriptEngine *engine = m_pEngine;
        m_pEngine = NULL;
        engine->deleteLater();
    }

    initializeScriptEngine();
    loadScriptFiles(m_lastConfigPath, pPreset->scriptFileNames);

    qDebug() << "Re-initializing scripts";
    initializeScripts(pPreset->scriptFunctionPrefixes);
}

/* -------- ------------------------------------------------------
   Purpose: Run the initialization function for each loaded script
                if it exists
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::initializeScripts(QList<QString> scriptFunctionPrefixes) {
    m_scriptFunctionPrefixes = scriptFunctionPrefixes;

    QScriptValueList args;
    args << QScriptValue(m_pController->getName());
    args << QScriptValue(m_bDebug);

    // Call the init method for all the prefixes.
    callFunctionOnObjects(m_scriptFunctionPrefixes, "init", args);

    emit(initialized());
}

/* -------- ------------------------------------------------------
   Purpose: Validate script syntax, then evaluate() it so the
            functions are registered & available for use.
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
bool ControllerEngine::evaluate(QString filepath) {
    QList<QString> dummy;
    bool ret = evaluate(filepath, dummy);

    return ret;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool ControllerEngine::execute(QString function) {
    if (m_pEngine == NULL)
        return false;

    QScriptValue scriptFunction = m_pEngine->evaluate(function);

    if (checkException())
        return false;

    if (!scriptFunction.isFunction())
        return false;

    scriptFunction.call(QScriptValue());
    if (checkException())
        return false;

    return true;
}


/* -------- ------------------------------------------------------
Purpose: Evaluate & run script code
Input:   'this' object if applicable, Code string
Output:  false if an exception
-------- ------------------------------------------------------ */
bool ControllerEngine::internalExecute(QScriptValue thisObject,
                                       QString scriptCode) {
    // A special version of safeExecute since we're evaluating strings, not actual functions
    //  (execute() would print an error that it's not a function every time a timer fires.)
    if (m_pEngine == NULL)
        return false;

    // Check syntax
    QScriptSyntaxCheckResult result = m_pEngine->checkSyntax(scriptCode);
    QString error = "";
    switch (result.state()) {
        case (QScriptSyntaxCheckResult::Valid): break;
        case (QScriptSyntaxCheckResult::Intermediate):
            error = "Incomplete code";
            break;
        case (QScriptSyntaxCheckResult::Error):
            error = "Syntax error";
            break;
    }
    if (error!="") {
        error = QString("%1: %2 at line %3, column %4 of script code:\n%5\n")
                .arg(error,
                     result.errorMessage(),
                     QString::number(result.errorLineNumber()),
                     QString::number(result.errorColumnNumber()),
                     scriptCode);

        if (m_bDebug) {
            qCritical() << "ControllerEngine:" << error;
        } else {
            scriptErrorDialog(error);
        }
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(scriptCode);

    if (checkException()) {
        qDebug() << "Exception";
        return false;
    }

    return internalExecute(thisObject, scriptFunction);
}

/* -------- ------------------------------------------------------
Purpose: Evaluate & run script code
Input:   'this' object if applicable, Code string
Output:  false if an exception
-------- ------------------------------------------------------ */
bool ControllerEngine::internalExecute(QScriptValue thisObject, QScriptValue functionObject) {
    if(m_pEngine == NULL)
        return false;

    // If it's not a function, we're done.
    if (!functionObject.isFunction()) {
        return false;
    }

    // If it does happen to be a function, call it.
    functionObject.call(thisObject);
    if (checkException()) {
        qDebug() << "Exception";
        return false;
    }

    return true;
}

/**-------- ------------------------------------------------------
   Purpose: Evaluate & call a script function with argument list
   Input:   Function name, argument list
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool ControllerEngine::execute(QString function, QScriptValueList args) {
    if(m_pEngine == NULL) {
        qDebug() << "ControllerEngine::execute: No script engine exists!";
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(function);

    if (checkException())
        return false;

    return execute(scriptFunction, args);
}

/**-------- ------------------------------------------------------
   Purpose: Evaluate & call a script function with argument list
   Input:   Function name, argument list
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool ControllerEngine::execute(QScriptValue functionObject, QScriptValueList args) {

    if(m_pEngine == NULL) {
        qDebug() << "ControllerEngine::execute: No script engine exists!";
        return false;
    }

    if (!functionObject.isFunction()) {
        qDebug() << "Not a function";
        return false;
    }
    QScriptValue rc = functionObject.call(m_pEngine->globalObject(), args);
    if (!rc.isValid()) {
        qDebug() << "QScriptValue is not a function or ...";
        return false;
    }

    if (checkException())
        return false;
    return true;
}
/**-------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, data string (e.g. device ID)
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool ControllerEngine::execute(QString function, QString data) {
    if (m_pEngine == NULL) {
        qDebug() << "ControllerEngine::execute: No script engine exists!";
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(function);

    if (checkException()) {
        qDebug() << "ControllerEngine::execute: Exception";
        return false;
    }

    if (!scriptFunction.isFunction()) {
        qDebug() << "ControllerEngine::execute: Not a function";
        return false;
    }

    QScriptValueList args;
    args << QScriptValue(data);

    return execute(scriptFunction, args);
}

/**-------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, ponter to data buffer, length of buffer
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool ControllerEngine::execute(QString function, const QByteArray data) {
    if (m_pEngine == NULL) {
        return false;
    }

    if (!m_pEngine->canEvaluate(function)) {
        qCritical() << "ControllerEngine: ?Syntax error in function" << function;
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(function);

    if (checkException())
        return false;

    return execute(scriptFunction, data);
}

/**-------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, ponter to data buffer, length of buffer
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool ControllerEngine::execute(QScriptValue function, const QByteArray data) {
    if (m_pEngine == NULL) {
        return false;
    }

    if (checkException())
        return false;
    if (!function.isFunction())
        return false;

    QScriptValueList args;
    args << QScriptValue(m_pBaClass->newInstance(data));
    args << QScriptValue(data.size());

    return execute(function, args);
}

/* -------- ------------------------------------------------------
   Purpose: Check to see if a script threw an exception
   Input:   QScriptValue returned from call(scriptFunctionName)
   Output:  true if there was an exception
   -------- ------------------------------------------------------ */
bool ControllerEngine::checkException() {
    if(m_pEngine == NULL) {
        return false;
    }

    if (m_pEngine->hasUncaughtException()) {
        QScriptValue exception = m_pEngine->uncaughtException();
        QString errorMessage = exception.toString();
        int line = m_pEngine->uncaughtExceptionLineNumber();
        QStringList backtrace = m_pEngine->uncaughtExceptionBacktrace();
        QString filename = exception.property("fileName").toString();

        QStringList error;
        error << (filename.isEmpty() ? "" : filename) << errorMessage << QString(line);
        m_scriptErrors.insert((filename.isEmpty() ? "passed code" : filename), error);

        QString errorText = QString(tr("Uncaught exception at line %1 in file %2: %3"))
                            .arg(QString::number(line),
                                (filename.isEmpty() ? "" : filename),
                                errorMessage);

        if (filename.isEmpty())
            errorText = QString(tr("Uncaught exception at line %1 in passed code: %2"))
                        .arg(QString::number(line), errorMessage);

        if (m_bDebug)
            qCritical() << "ControllerEngine:" << errorText
                        << "\nBacktrace:\n"
                        << backtrace;
        else scriptErrorDialog(errorText);
        return true;
    }
    return false;
}

/*  -------- ------------------------------------------------------
    Purpose: Common error dialog creation code for run-time exceptions
                Allows users to ignore the error or reload the mappings
    Input:   Detailed error string
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scriptErrorDialog(QString detailedError) {
    qWarning() << "ControllerEngine:" << detailedError;
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_WARNING);
    props->setTitle(tr("Controller script error"));
    props->setText(tr("A control you just used is not working properly."));
    props->setInfoText(tr("<html>(The script code needs to be fixed.)"
        "<br>For now, you can:<ul><li>Ignore this error for this session but you may experience erratic behavior</li>"
        "<li>Try to recover by resetting your controller</li></ul></html>"));
    props->setDetails(detailedError);
    props->setKey(detailedError);   // To prevent multiple windows for the same error

    // Allow user to suppress further notifications about this particular error
    props->addButton(QMessageBox::Ignore);

    props->addButton(QMessageBox::Retry);
    props->addButton(QMessageBox::Close);
    props->setDefaultButton(QMessageBox::Close);
    props->setEscapeButton(QMessageBox::Close);
    props->setModal(false);

    if (ErrorDialogHandler::instance()->requestErrorDialog(props)) {
        // Enable custom handling of the dialog buttons
        connect(ErrorDialogHandler::instance(), SIGNAL(stdButtonClicked(QString, QMessageBox::StandardButton)),
                this, SLOT(errorDialogButton(QString, QMessageBox::StandardButton)));
    }
}

/* -------- ------------------------------------------------------
    Purpose: Slot to handle custom button clicks in error dialogs
    Input:   Key of dialog, StandardButton that was clicked
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::errorDialogButton(QString key, QMessageBox::StandardButton button) {
    Q_UNUSED(key);

    // Something was clicked, so disable this signal now
    disconnect(ErrorDialogHandler::instance(),
               SIGNAL(stdButtonClicked(QString, QMessageBox::StandardButton)),
               this,
               SLOT(errorDialogButton(QString, QMessageBox::StandardButton)));

    if (button == QMessageBox::Retry) {
        emit(resetController());
    }
}

ControlObjectThread* ControllerEngine::getControlObjectThread(QString group, QString name) {
    ConfigKey key = ConfigKey(group, name);

    ControlObjectThread *cot = NULL;
    if(!m_controlCache.contains(key)) {
        ControlObject *co = ControlObject::getControl(key);
        if(co != NULL) {
            cot = new ControlObjectThread(co);
            m_controlCache.insert(key, cot);
        }
    } else {
        cot = m_controlCache.value(key);
    }

    return cot;
}

/* -------- ------------------------------------------------------
   Purpose: Returns the current value of a Mixxx control (for scripts)
   Input:   Control group (e.g. [Channel1]), Key name (e.g. [filterHigh])
   Output:  The value
   -------- ------------------------------------------------------ */
double ControllerEngine::getValue(QString group, QString name) {
    ControlObjectThread *cot = getControlObjectThread(group, name);
    if (cot == NULL) {
        qWarning() << "ControllerEngine: Unknown control" << group << name << ", returning 0.0";
        return 0.0;
    }
    return cot->get();
}

/* -------- ------------------------------------------------------
   Purpose: Sets new value of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::setValue(QString group, QString name, double newValue) {
    if (isnan(newValue)) {
        qWarning() << "ControllerEngine: script setting [" << group << "," << name
                 << "] to NotANumber, ignoring.";
        return;
    }

    ControlObjectThread *cot = getControlObjectThread(group, name);

    if (cot != NULL && !m_st.ignore(cot->getControlObject(), newValue)) {
        cot->slotSet(newValue);
        // We call emitValueChanged so that script functions connected to this
        // control will get updates.
        cot->emitValueChanged();
    }
}

/* -------- ------------------------------------------------------
   Purpose: qDebugs script output so it ends up in mixxx.log
   Input:   String to log
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::log(QString message) {
    qDebug() << message;
}

/* -------- ------------------------------------------------------
   Purpose: Emits valueChanged() so device outputs update
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::trigger(QString group, QString name) {
    ControlObjectThread *cot = getControlObjectThread(group, name);
    if(cot != NULL) {
        cot->emitValueChanged();
    }
}

/**-------- ------------------------------------------------------
   Purpose: (Dis)connects a ControlObject valueChanged() signal to/from a script function
   Input:   Control group (e.g. [Channel1]), Key name (e.g. [filterHigh]),
                script function name, true if you want to disconnect
   Output:  true if successful
   -------- ------------------------------------------------------ */
QScriptValue ControllerEngine::connectControl(QString group, QString name,
                                              QScriptValue callback, bool disconnect) {
    ConfigKey key(group, name);
    ControlObjectThread* cot = getControlObjectThread(group, name);
    QScriptValue function;

    if (cot == NULL) {
        qWarning() << "ControllerEngine: script connecting [" << group << "," << name
                   << "], which is non-existent. ignoring.";
        return QScriptValue();
    }

    if (m_pEngine == NULL) {
        return QScriptValue(FALSE);
    }

    if (callback.isString()) {
        ControllerEngineConnection cb;
        cb.key = key;
        cb.id = callback.toString();

        if (disconnect) {
            disconnectControl(cb);
            return QScriptValue(TRUE);
        }

        function = m_pEngine->evaluate(callback.toString());
        if (checkException() || !function.isFunction()) {
            return QScriptValue(FALSE);
        } else if (m_connectedControls.contains(key, cb)) {
            // Do not allow multiple connections to named functions

            // Return a wrapper to the conn
            QHash<ConfigKey, ControllerEngineConnection>::iterator i =
                m_connectedControls.find(key);

            ControllerEngineConnection conn = i.value();
            return m_pEngine->newQObject(
                        new ControllerEngineConnectionScriptValue(conn),
                        QScriptEngine::ScriptOwnership
                    );
        }
    }
    else if (callback.isFunction()) {
        function = callback;
    }
    // Assume a ControllerEngineConnection
    else if (callback.isQObject()) {
        QObject *qobject = callback.toQObject();
        const QMetaObject *qmeta = qobject->metaObject();

        if (!strcmp(qmeta->className(), "ControllerEngineConnectionScriptValue")) {
            ControllerEngineConnectionScriptValue *proxy =
                (ControllerEngineConnectionScriptValue *)qobject;
            proxy->disconnect();
        }
    }
    else {
        qWarning() << "Invalid callback";
        return QScriptValue(FALSE);
    }

    if (function.isFunction()) {
        qDebug() << "Connection:" << group << name;
        connect(cot, SIGNAL(valueChanged(double)),
                this, SLOT(slotValueChanged(double)),
                Qt::QueuedConnection);

        ControllerEngineConnection conn;
        conn.key = key;
        conn.ce = this;
        conn.function = function;

        QScriptContext *ctxt = m_pEngine->currentContext();
        // Our current context is a function call to engine.connectControl. We
        // want to grab the 'this' from the caller's context, so we walk up the
        // stack.
        if (ctxt) {
            ctxt = ctxt->parentContext();
            conn.context = ctxt ? ctxt->thisObject() : QScriptValue();
        }

        if (callback.isString()) {
            conn.id = callback.toString();
        }
        else {
            QUuid uuid = QUuid::createUuid();
            conn.id = uuid.toString();
        }

        m_connectedControls.insert(key, conn);
        return m_pEngine->newQObject(
                    new ControllerEngineConnectionScriptValue(conn),
                    QScriptEngine::ScriptOwnership
                );
    }

    return QScriptValue(FALSE);
}

/* -------- ------------------------------------------------------
   Purpose: (Dis)connects a ControlObject valueChanged() signal to/from a script function
   Input:   Control group (e.g. [Channel1]), Key name (e.g. [filterHigh]),
                script function name, true if you want to disconnect
   Output:  true if successful
   -------- ------------------------------------------------------ */
void ControllerEngine::disconnectControl(const ControllerEngineConnection conn) {
    ControlObjectThread* cot = getControlObjectThread(conn.key.group, conn.key.item);

    if(m_pEngine == NULL) {
        return;
    }

    if (m_connectedControls.contains(conn.key, conn)) {
        m_connectedControls.remove(conn.key, conn);
        // Only disconnect the signal if there are no other instances of this control using it
        if (!m_connectedControls.contains(conn.key)) {
            disconnect(cot, SIGNAL(valueChanged(double)),
                       this, SLOT(slotValueChanged(double)));
        }
    } else {
        qWarning() << "Could not Disconnect connection" << conn.id;
    }
}

void ControllerEngineConnectionScriptValue::disconnect() {
    conn.ce->disconnectControl(conn);
}

/**-------- ------------------------------------------------------
   Purpose: Receives valueChanged() slots from ControlObjects, and
   fires off the appropriate script function.
   -------- ------------------------------------------------------ */
void ControllerEngine::slotValueChanged(double value) {
    ControlObjectThread* senderCOT = dynamic_cast<ControlObjectThread*>(sender());
    if (senderCOT == NULL) {
        qWarning() << "ControllerEngine::slotValueChanged() Shouldn't happen -- sender == NULL";
        return;
    }

    ControlObject* pSenderCO = senderCOT->getControlObject();
    if (pSenderCO == NULL) {
        qWarning() << "ControllerEngine::slotValueChanged() The sender's CO is NULL.";
        return;
    }
    ConfigKey key = pSenderCO->getKey();

//     qDebug() << "[Controller]: SlotValueChanged" << key.group << key.item;

    if (m_connectedControls.contains(key)) {
        QHash<ConfigKey, ControllerEngineConnection>::iterator iter =
            m_connectedControls.find(key);
        QList<ControllerEngineConnection> conns;

        // Create a temporary list to allow callbacks to disconnect
        // -Phillip Whelan
        while (iter != m_connectedControls.end() && iter.key() == key) {
            conns.append(iter.value());
            ++iter;
        }

        for (int i = 0; i < conns.size(); i++) {
            ControllerEngineConnection conn = conns.at(i);
            QScriptValueList args;

            args << QScriptValue(value);
            args << QScriptValue(key.group);
            args << QScriptValue(key.item);
            QScriptValue result = conn.function.call(conn.context, args);
            if (result.isError()) {
                qWarning()<< "ControllerEngine: Call to callback" << conn.id
                          << "resulted in an error:" << result.toString();
            }
        }
    } else {
        qWarning() << "ControllerEngine::slotValueChanged() Received signal from ControlObject that is not connected to a script function.";
    }
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate a script file
   Input:   Script filename
   Output:  false if the script file has errors or doesn't exist
   -------- ------------------------------------------------------ */
bool ControllerEngine::evaluate(QString scriptName, QList<QString> scriptPaths) {
    if (m_pEngine == NULL) {
        return false;
    }

    QString filename = "";
    QFile input;

    if (scriptPaths.length() == 0) {
        // If we aren't given any paths to search, assume that scriptName
        // contains the full file name
        filename = scriptName;
        input.setFileName(filename);
    } else {
        foreach (QString scriptPath, scriptPaths) {
            QDir scriptPathDir(scriptPath);
            filename = scriptPathDir.absoluteFilePath(scriptName);
            input.setFileName(filename);
            if (input.exists())  {
                qDebug() << "ControllerEngine: Watching JS File:" << filename;
                m_scriptWatcher.addPath(filename);
                break;
            }
        }
    }

    qDebug() << "ControllerEngine: Loading" << filename;

    // Read in the script file
    if (!input.open(QIODevice::ReadOnly)) {
        QString errorLog =
            QString("ControllerEngine: Problem opening the script file: %1, error # %2, %3")
                .arg(filename, QString("%1").arg(input.error()), input.errorString());

        if (m_bDebug) {
            qCritical() << errorLog;
        } else {
            qWarning() << errorLog;
            if (m_bPopups) {
                // Set up error dialog
                ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
                props->setType(DLG_WARNING);
                props->setTitle("Controller script file problem");
                props->setText(QString("There was a problem opening the controller script file %1.").arg(filename));
                props->setInfoText(input.errorString());

                // Ask above layer to display the dialog & handle user response
                ErrorDialogHandler::instance()->requestErrorDialog(props);
            }
        }
        return false;
    }

    QString scriptCode = "";
    scriptCode.append(input.readAll());
    scriptCode.append('\n');
    input.close();

    // Check syntax
    QScriptSyntaxCheckResult result = m_pEngine->checkSyntax(scriptCode);
    QString error="";
    switch (result.state()) {
        case (QScriptSyntaxCheckResult::Valid): break;
        case (QScriptSyntaxCheckResult::Intermediate):
            error = "Incomplete code";
            break;
        case (QScriptSyntaxCheckResult::Error):
            error = "Syntax error";
            break;
    }
    if (error != "") {
        error = QString("%1 at line %2, column %3 in file %4: %5")
                    .arg(error,
                         QString::number(result.errorLineNumber()),
                         QString::number(result.errorColumnNumber()),
                         filename, result.errorMessage());

        if (m_bDebug) {
            qCritical() << "ControllerEngine:" << error;
        } else {
            qWarning() << "ControllerEngine:" << error;
            if (m_bPopups) {
                ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
                props->setType(DLG_WARNING);
                props->setTitle("Controller script file error");
                props->setText(QString("There was an error in the controller script file %1.").arg(filename));
                props->setInfoText("The functionality provided by this script file will be disabled.");
                props->setDetails(error);

                ErrorDialogHandler::instance()->requestErrorDialog(props);
            }
        }
        return false;
    }

    // Evaluate the code
    QScriptValue scriptFunction = m_pEngine->evaluate(scriptCode, filename);

    // Record errors
    if (checkException()) {
        return false;
    }

    return true;
}

/*
 * Check whether a source file that was evaluated()'d has errors.
 */
bool ControllerEngine::hasErrors(QString filename) {
    bool ret = m_scriptErrors.contains(filename);
    return ret;
}

/*
 * Get the errors for a source file that was evaluated()'d
 */
const QStringList ControllerEngine::getErrors(QString filename) {
    QStringList ret = m_scriptErrors.value(filename, QStringList());
    return ret;
}


/* -------- ------------------------------------------------------
   Purpose: Creates & starts a timer that runs some script code
                on timeout
   Input:   Number of milliseconds, script function to call,
                whether it should fire just once
   Output:  The timer's ID, 0 if starting it failed
   -------- ------------------------------------------------------ */
int ControllerEngine::beginTimer(int interval, QScriptValue timerCallback,
                                 bool oneShot) {
    if (!timerCallback.isFunction() && !timerCallback.isString()) {
        qWarning() << "Invalid timer callback provided to beginTimer."
                   << "Valid callbacks are strings and functions.";
        return 0;
    }

    if (interval < 20) {
        qWarning() << "Timer request for" << interval
                   << "ms is too short. Setting to the minimum of 20ms.";
        interval = 20;
    }

    // This makes use of every QObject's internal timer mechanism. Nice, clean,
    // and simple. See http://doc.trolltech.com/4.6/qobject.html#startTimer for
    // details
    int timerId = startTimer(interval);
    TimerInfo info;
    info.callback = timerCallback;
    QScriptContext *ctxt = m_pEngine->currentContext();
    info.context = ctxt ? ctxt->thisObject() : QScriptValue();
    info.oneShot = oneShot;
    m_timers[timerId] = info;
    if (timerId == 0) {
        qWarning() << "Script timer could not be created";
    } else if (m_bDebug) {
        if (oneShot)
            qDebug() << "Starting one-shot timer:" << timerId;
        else
            qDebug() << "Starting timer:" << timerId;
    }
    return timerId;
}

/* -------- ------------------------------------------------------
   Purpose: Stops & removes a timer
   Input:   ID of timer to stop
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::stopTimer(int timerId) {
    if (!m_timers.contains(timerId)) {
        qWarning() << "Killing timer" << timerId << ": That timer does not exist!";
        return;
    }
    if (m_bDebug) {
        qDebug() << "Killing timer:" << timerId;
    }

    killTimer(timerId);
    m_timers.remove(timerId);
}

/* -------- ------------------------------------------------------
   Purpose: Stops & removes all timers (for shutdown)
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::stopAllTimers() {
    QMutableHashIterator<int, TimerInfo> i(m_timers);
    while (i.hasNext()) {
        i.next();
        stopTimer(i.key());
    }
}

/* -------- ------------------------------------------------------
   Purpose: Runs the appropriate script code on timer events
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void ControllerEngine::timerEvent(QTimerEvent *event) {
    int timerId = event->timerId();

    // See if this is a scratching timer
    if (m_scratchTimers.contains(timerId)) {
        scratchProcess(timerId);
        return;
    }

    if (!m_timers.contains(timerId)) {
        qWarning() << "Timer" << timerId << "fired but there's no function mapped to it!";
        return;
    }

    TimerInfo timerTarget = m_timers[timerId];
    if (timerTarget.oneShot) {
        stopTimer(timerId);
    }

    if (timerTarget.callback.isString()) {
        internalExecute(timerTarget.context, timerTarget.callback.toString());
    } else if (timerTarget.callback.isFunction()) {
        internalExecute(timerTarget.context, timerTarget.callback);
    }
}

/* -------- ------------------------------------------------------
    Purpose: Enables scratching for relative controls
    Input:   Virtual deck to scratch,
             Number of intervals per revolution of the controller wheel,
             RPM for the track at normal speed (usually 33+1/3),
             (optional) alpha value for the filter,
             (optional) beta value for the filter
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scratchEnable(int deck, int intervalsPerRev, float rpm,
                                     float alpha, float beta, bool ramp) {

    // If we're already scratching this deck, override that with this request
    if (m_dx[deck]) {
        //qDebug() << "Already scratching deck" << deck << ". Overriding.";
        int timerId = m_scratchTimers.key(deck);
        killTimer(timerId);
        m_scratchTimers.remove(timerId);
    }

    // Controller resolution in intervals per second at normal speed (rev/min * ints/rev * mins/sec)
    float intervalsPerSecond = (rpm * intervalsPerRev)/60;

    m_dx[deck] = 1/intervalsPerSecond;
    m_intervalAccumulator[deck] = 0;
    m_ramp[deck] = false;
    m_rampFactor[deck] = 0.001;
    m_brakeActive[deck] = false;

    QString group = QString("[Channel%1]").arg(deck);

    // Ramp
    float initVelocity = 0.0;   // Default to stopped
    ControlObjectThread *cot = getControlObjectThread(group, "scratch2_enable");

    // If ramping is desired, figure out the deck's current speed
    if (ramp) {
        // See if the deck is already being scratched
        if (cot != NULL && cot->get() == 1) {
            // If so, set the filter's initial velocity to the scratch speed
            cot = getControlObjectThread(group, "scratch2");
            if (cot != NULL) {
                initVelocity=cot->get();
            }
        } else {
            // See if deck is playing
            cot = getControlObjectThread(group, "play");
            if (cot != NULL && cot->get() == 1) {
                // If so, set the filter's initial velocity to the playback speed
                float rate = 0;

                cot = getControlObjectThread(group, "rate");
                if (cot != NULL) {
                    rate = cot->get();
                }

                cot = getControlObjectThread(group, "rateRange");
                if (cot != NULL) {
                    rate = rate * cot->get();
                }

                // Add 1 since the deck is playing
                rate++;

                // See if we're in reverse play
                cot = getControlObjectThread(group, "reverse");

                if (cot != NULL && cot->get() == 1) {
                    rate = -rate;
                }
                initVelocity = rate;
            }
        }
    }

    // Initialize pitch filter (0.001s = 1ms) (We're assuming the OS actually
    // gives us a 1ms timer below)
    if (alpha && beta) {
        m_pitchFilter[deck]->init(0.001, initVelocity, alpha, beta);
    } else {
        // Use filter's defaults if not specified
        m_pitchFilter[deck]->init(0.001, initVelocity);
    }

    // 1ms is shortest possible, OS dependent
    int timerId = startTimer(1);

    // Associate this virtual deck with this timer for later processing
    m_scratchTimers[timerId] = deck;

    // Set scratch2_enable
    cot = getControlObjectThread(group, "scratch2_enable");
    if(cot != NULL) {
        cot->slotSet(1);
    }
}

/* -------- ------------------------------------------------------
    Purpose: Accumulates "ticks" of the controller wheel
    Input:   Virtual deck to scratch, interval value (usually +1 or -1)
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scratchTick(int deck, int interval) {
    m_lastMovement[deck] = SoftTakeover::currentTimeMsecs();
    m_intervalAccumulator[deck] += interval;
}

/* -------- ------------------------------------------------------
    Purpose: Applies the accumulated movement to the track speed
    Input:   ID of timer for this deck
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scratchProcess(int timerId) {
    int deck = m_scratchTimers[timerId];
    QString group = QString("[Channel%1]").arg(deck);
    PitchFilter* filter = m_pitchFilter[deck];
    if (!filter) {
        qWarning() << "Scratch filter pointer is null on deck" << deck;
        return;
    }

    const float oldPitch = filter->currentPitch();

    // Give the filter a data point:

    // If we're ramping to end scratching
    //  and the wheel hasn't been turned very recently (spinback after lift-off,)
    //  feed fixed data
    if (m_ramp[deck] &&
        ((SoftTakeover::currentTimeMsecs() - m_lastMovement[deck]) > 0)) {
        filter->observation(m_rampTo[deck]*m_rampFactor[deck]);
        // Once this code path is run, latch so it always runs until reset
//         m_lastMovement[deck] += 1000;
    } else {
        //  This will (and should) be 0 if no net ticks have been accumulated
        //  (i.e. the wheel is stopped)
        filter->observation(m_dx[deck] * m_intervalAccumulator[deck]);
    }

    const float newPitch = filter->currentPitch();

    // Actually do the scratching
    ControlObjectThread *cot = getControlObjectThread(group, "scratch2");
    if(cot == NULL) {
        return; // abort and maybe it'll work on the next pass
    }
    cot->slotSet(filter->currentPitch());

    // Reset accumulator
    m_intervalAccumulator[deck] = 0;

    //if (m_ramp[deck]) qDebug() << "Ramping to" << m_rampTo[deck] << " Currently at:" << filter->currentPitch();

    // If we're ramping and the current pitch is really close to the rampTo
    // value or we're in brake mode and have crossed over the zero value, end scratching
    if ((m_ramp[deck] && fabs(m_rampTo[deck] - newPitch) <= 0.00001) ||
        (m_brakeActive[deck] && (
            (oldPitch > 0.0 && newPitch < 0.0) ||
            (oldPitch < 0.0 && newPitch > 0.0)))) {
        // Not ramping no mo'
        m_ramp[deck] = false;

        // If in brake mode, we just set scratch2 to 0.0 (then wait until someone
        //  calls scratchDisable().)
        if (m_brakeActive[deck]) {
            cot->slotSet(0.0);
        } else {
            // Clear scratch2_enable to end scratching
            cot = getControlObjectThread(group, "scratch2_enable");
            if(cot == NULL) {
                return; // abort and maybe it'll work on the next pass
            }
            cot->slotSet(0);
        }

        // Remove timer
        killTimer(timerId);
        m_scratchTimers.remove(timerId);

        m_dx[deck] = 0.0;
        m_brakeActive[deck] = false;
    }
}

/* -------- ------------------------------------------------------
    Purpose: Stops scratching the specified virtual deck
    Input:   Virtual deck to stop scratching
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::scratchDisable(int deck, bool ramp) {
    QString group = QString("[Channel%1]").arg(deck);

    m_rampTo[deck] = 0.0;

    // If no ramping is desired, disable scratching immediately
    if (!ramp) {
        // Clear scratch2_enable
        ControlObjectThread *cot = getControlObjectThread(group, "scratch2_enable");
        if(cot != NULL) cot->slotSet(0);
        // Can't return here because we need scratchProcess to stop the timer.
        //  So it's still actually ramping, we just won't hear or see it.
    } else {
        // See if deck is playing
        ControlObjectThread *cot = getControlObjectThread(group, "play");
        if (cot != NULL && cot->get() == 1) {
            // If so, set the target velocity to the playback speed
            float rate=0;
            // Get the pitch slider value
            cot = getControlObjectThread(group, "rate");
            if (cot != NULL) {
                rate = cot->get();
            }

            // Get the pitch slider direction
            cot = getControlObjectThread(group, "rate_dir");
            if (cot != NULL && cot->get() == -1) {
                rate = -rate;
            }

            // Multiply by the pitch range
            cot = getControlObjectThread(group, "rateRange");
            if (cot != NULL) {
                rate = rate * cot->get();
            }

            // Add 1 since the deck is playing
            rate++;

            // See if we're in reverse play
            cot = getControlObjectThread(group, "reverse");
            if (cot != NULL && cot->get() == 1) {
                rate = -rate;
            }

            m_rampTo[deck] = rate;
        }
    }

    m_lastMovement[deck] = SoftTakeover::currentTimeMsecs();
    m_ramp[deck] = true;    // Activate the ramping in scratchProcess()
}

/* -------- ------------------------------------------------------
    Purpose: Tells if the specified deck is currently scratching
             (Scripts need this to implement spinback-after-lift-off)
    Input:   Virtual deck to inquire about
    Output:  True if so
    -------- ------------------------------------------------------ */
bool ControllerEngine::isScratching(int deck) {
    QString group = QString("[Channel%1]").arg(deck);
    return getValue(group,"scratch2_enable")>0;
}

/*  -------- ------------------------------------------------------
    Purpose: [En/dis]ables soft-takeover status for a particular ControlObject
    Input:   ControlObject group and key values,
                whether to set the soft-takeover status or not
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::softTakeover(QString group, QString name, bool set) {
    ControlObject* pControl = ControlObject::getControl(ConfigKey(group, name));
    if (!pControl) {
        return;
    }
    if (set) {
        m_st.enable(pControl);
    } else {
        m_st.disable(pControl);
    }
}

/*  -------- ------------------------------------------------------
    Purpose: [En/dis]ables spinback effect for the channel
    Input:   deck, activate/deactivate, factor (optional),
             delay (optional), rate (optional)
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::spinback(int deck, bool activate, float factor, float rate) {
    // defaults for args set in header file
    brake(deck, activate, factor, rate);
}

/*  -------- ------------------------------------------------------
    Purpose: [En/dis]ables brake/spinback effect for the channel
    Input:   deck, activate/deactivate, factor (optional),
             delay (optional), rate (optional)
    Output:  -
    -------- ------------------------------------------------------ */
void ControllerEngine::brake(int deck, bool activate, float factor, float rate) {
    QString group = QString("[Channel%1]").arg(deck);

    // kill timer when both enabling or disabling
    int timerId = m_scratchTimers.key(deck);
    killTimer(timerId);
    m_scratchTimers.remove(timerId);

    // enable/disable scratch2 mode
    ControlObjectThread *cot = getControlObjectThread(group, "scratch2_enable");
    if (cot != NULL) {
        cot->slotSet(activate ? 1 : 0);
    }

    // used in scratchProcess for the different timer behaviour we need
    m_brakeActive[deck] = activate;

    if (activate) {
        // store the new values for this spinback/brake effect
        m_rampFactor[deck] = rate * factor / 100000; // approx 1 second for a factor of 1
        m_rampTo[deck] = -1.0;

        // save current keylock status and disable
        cot = getControlObjectThread(group, "keylock");
        if (cot != NULL) {
            m_brakeKeylock[deck] = cot->get();
            cot->slotSet(0);
        }

        // setup timer and send first scratch2 'tick'
        int timerId = startTimer(1);
        m_scratchTimers[timerId] = deck;

        cot = getControlObjectThread(group, "scratch2");
        if (cot != NULL) {
            cot->slotSet(rate);
        }

        // setup the filter
        PitchFilter* filter = m_pitchFilter[deck];
        if (filter != NULL) {
            m_pitchFilter[deck]->init(0.001, rate);
        }

        // activate the ramping in scratchProcess()
        m_ramp[deck] = true;
    }
    else {
        // re-enable keylock if needed
        if (m_brakeKeylock[deck]) {
            cot = getControlObjectThread(group, "keylock");
            if (cot != NULL) {
                cot->slotSet(1);
            }
        }
    }
}
