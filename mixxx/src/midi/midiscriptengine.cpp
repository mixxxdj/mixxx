/***************************************************************************
                          midiscriptengine.cpp  -  description
                          -------------------
    begin                : Fri Dec 12 2008
    copyright            : (C) 2008-2010 by Sean M. Pappalardo
                                       "Holy crap, I wrote new code!"
    email                : spappalardo@mixxx.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "controlobject.h"
#include "controlobjectthread.h"
#include "mididevice.h"
#include "midiscriptengine.h"
#include "errordialoghandler.h"

// #include <QScriptSyntaxCheckResult>

#ifdef _MSC_VER
    #include <float.h>  // for _isnan() on VC++
    #define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#else
    #include <math.h>  // for isnan() everywhere else
#endif


MidiScriptEngine::MidiScriptEngine(MidiDevice* midiDevice) :
    m_pMidiDevice(midiDevice),
    m_midiDebug(false),
    m_pEngine(NULL),
    m_midiPopups(false) {

    // Handle error dialog buttons
    qRegisterMetaType<QMessageBox::StandardButton>("QMessageBox::StandardButton");

    // Pre-allocate arrays for average number of virtual decks
    int decks = 16;
    m_intervalAccumulator.resize(decks);
    m_dx.resize(decks);
    m_rampTo.resize(decks);
    m_ramp.resize(decks);
    m_pitchFilter.resize(decks);

    // Initialize arrays used for testing and pointers
    for (int i=0; i < decks; i++) {
        m_dx[i] = 0;
        m_pitchFilter[i] = new PitchFilter(); // allocate RAM at startup
        m_ramp[i] = false;
    }
}

MidiScriptEngine::~MidiScriptEngine() {
    // Clean up
    int decks = 16; // Must match value above
    for (int i=0; i < decks; i++) {
        delete m_pitchFilter[i];
        m_pitchFilter[i] = NULL;
    }

    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if(m_pEngine != NULL) {
        QScriptEngine *engine = m_pEngine;
        m_pEngine = NULL;
        engine->deleteLater();
    }

}

/* -------- ------------------------------------------------------
Purpose: Shuts down MIDI scripts in an orderly fashion
            (stops timers then executes shutdown functions)
Input:   -
Output:  -
-------- ------------------------------------------------------ */
void MidiScriptEngine::gracefulShutdown(QList<QString> scriptFunctionPrefixes) {
    qDebug() << "MidiScriptEngine shutting down...";

    m_scriptEngineLock.lock();
    // Clear the m_connectedControls hash so we stop responding
    // to signals.
    m_connectedControls.clear();

    // Disconnect the function call signal
    if (m_pMidiDevice)
        disconnect(m_pMidiDevice, SIGNAL(callMidiScriptFunction(QString, char, char,
                                                                char, MidiStatusByte, QString)),
                   this, SLOT(execute(QString, char, char, char, MidiStatusByte, QString)));

    // Stop all timers
    stopAllTimers();

    // Call each script's shutdown function if it exists
    QListIterator<QString> prefixIt(scriptFunctionPrefixes);
    while (prefixIt.hasNext()) {
        QString shutName = prefixIt.next();
        if (shutName!="") {
            shutName.append(".shutdown");
            if (m_midiDebug) qDebug() << "MidiScriptEngine: Executing" << shutName;
            if (!internalExecute(shutName))
                qWarning() << "MidiScriptEngine: No" << shutName << "function in script";
        }
    }

    // Prevents leaving decks in an unstable state
    //  if the controller is shut down while scratching
    QHashIterator<int, int> i(m_scratchTimers);
    while (i.hasNext()) {
        i.next();
        qDebug() << "Aborting scratching on deck" << i.value();
        // Clear scratch2_enable
        QString group = QString("[Channel%1]").arg(i.value());
        ControlObjectThread *cot = getControlObjectThread(group, "scratch2_enable");
        if(cot != NULL) cot->slotSet(0);
    }

    // Free all the control object threads
    QList<ConfigKey> keys = m_controlCache.keys();
    QList<ConfigKey>::iterator it = keys.begin();
    QList<ConfigKey>::iterator end = keys.end();
    while(it != end) {
        ConfigKey key = *it;
        ControlObjectThread *cot = m_controlCache.take(key);
        delete cot;
        it++;
    }

    m_scriptEngineLock.unlock();

    // Stop processing the event loop and terminate the thread.
    quit();
}

bool MidiScriptEngine::isReady() {
    m_scriptEngineLock.lock();
    bool ret = m_pEngine != NULL;
    m_scriptEngineLock.unlock();
    return ret;
}

/*
  WARNING: must hold the lock to call this
 */
void MidiScriptEngine::initializeScriptEngine() {
    // Create the MidiScriptEngine
    m_pEngine = new QScriptEngine(this);

    //qDebug() << "MidiScriptEngine::run() m_pEngine->parent() is " << m_pEngine->parent();
    //qDebug() << "MidiScriptEngine::run() m_pEngine->thread() is " << m_pEngine->thread();

    // Make this MidiScriptEngine instance available to scripts as
    // 'engine'.
    QScriptValue engineGlobalObject = m_pEngine->globalObject();
    engineGlobalObject.setProperty("engine", m_pEngine->newQObject(this));

    if (m_pMidiDevice) {
        qDebug() << "MIDI Device in script engine is:" << m_pMidiDevice->getName();

        // Make the MidiDevice instance available to scripts as 'midi'.
        engineGlobalObject.setProperty("midi", m_pEngine->newQObject(m_pMidiDevice));

        // Allow the MidiDevice to signal script function calls
        connect(m_pMidiDevice, SIGNAL(callMidiScriptFunction(QString, char, char,
                                                             char, MidiStatusByte, QString)),
                this, SLOT(execute(QString, char, char, char, MidiStatusByte, QString)));
    }
}

/* -------- ------------------------------------------------------
   Purpose: Load all script files given in the shared list
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::loadScriptFiles(QList<QString> scriptFileNames) {

    // Set the Midi Debug flag
    if (m_pMidiDevice)
        m_midiDebug = m_pMidiDevice->midiDebugging();

    qDebug() << "MidiScriptEngine: Loading & evaluating all MIDI script code";

    // scriptPaths holds the paths to search in when we're looking for scripts
    QList<QString> scriptPaths;
    scriptPaths.append(QDir::homePath().append("/").append(SETTINGS_PATH).append("presets/"));

    ConfigObject<ConfigValue> *config = new ConfigObject<ConfigValue>(QDir::homePath().append("/").append(SETTINGS_PATH).append(SETTINGS_FILE));
    scriptPaths.append(config->getConfigPath().append("midi/"));
    delete config;

    QListIterator<QString> it(scriptFileNames);
    m_scriptEngineLock.lock();
    while (it.hasNext()) {
        QString curScriptFileName = it.next();
        safeEvaluate(curScriptFileName, scriptPaths);

        if(m_scriptErrors.contains(curScriptFileName)) {
            qDebug() << "Errors occured while loading " << curScriptFileName;
        }
    }

    m_scriptEngineLock.unlock();
    emit(initialized());
}

/* -------- ------------------------------------------------------
   Purpose: Run the initialization function for each loaded script
                if it exists
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::initializeScripts(QList<QString> scriptFunctionPrefixes) {
    m_scriptEngineLock.lock();

    QListIterator<QString> prefixIt(scriptFunctionPrefixes);
    while (prefixIt.hasNext()) {
        QString initName = prefixIt.next();
            if (initName!="") {
                initName.append(".init");
            if (m_midiDebug) qDebug() << "MidiScriptEngine: Executing" << initName;
            if (!safeExecute(initName, m_pMidiDevice->getName()))
                qWarning() << "MidiScriptEngine: No" << initName << "function in script";
        }
    }
    m_scriptEngineLock.unlock();
    emit(initialized());
}

/* -------- ------------------------------------------------------
   Purpose: Create the MidiScriptEngine object (so it is owned in this
   thread, and start the Qt event loop for this thread via exec().
   Input: -
   Output: -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::run() {
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("MidiScriptEngine %1").arg(++id));

    // Prevent the script engine from strangling other parts of Mixxx
    //  incase of a misbehaving script
    //  - Should we perhaps not do this when running with --midiDebug so it's more
    //      obvious if a script is taking too much CPU time? - Sean 4/19/10
    QThread::currentThread()->setPriority(QThread::LowPriority);

    m_scriptEngineLock.lock();
    initializeScriptEngine();
    m_scriptEngineLock.unlock();
    emit(initialized());

    // Run the Qt event loop indefinitely
    exec();
}

/* -------- ------------------------------------------------------
   Purpose: Validate script syntax, then evaluate() it so the
            functions are registered & available for use.
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::evaluate(QString filepath) {
    m_scriptEngineLock.lock();
    QList<QString> dummy;
    bool ret = safeEvaluate(filepath, dummy);
    m_scriptEngineLock.unlock();
    return ret;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::execute(QString function) {
    m_scriptEngineLock.lock();
    bool ret = safeExecute(function);
    if (!ret) qWarning() << "MidiScriptEngine: Invalid script function" << function;
    m_scriptEngineLock.unlock();
    return ret;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, data string (e.g. device ID)
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::execute(QString function, QString data) {
    m_scriptEngineLock.lock();
    bool ret = safeExecute(function, data);
    if (!ret) qWarning() << "MidiScriptEngine: Invalid script function" << function;
    m_scriptEngineLock.unlock();
    return ret;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, pointer to data buffer, length of buffer
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::execute(QString function, const unsigned char data[],
                               unsigned int length) {
    m_scriptEngineLock.lock();
    bool ret = safeExecute(function, data, length);
    m_scriptEngineLock.unlock();
    return ret;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, channel #, control #, value, status
                MixxxControl group
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::execute(QString function, char channel,
                               char control, char value,
                               MidiStatusByte status,
                               QString group) {
    m_scriptEngineLock.lock();
    bool ret = safeExecute(function, channel, control, value, status, group);
    if (!ret) qWarning() << "MidiScriptEngine: Invalid script function" << function;
    m_scriptEngineLock.unlock();
    return ret;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeExecute(QString function) {
    //qDebug() << QString("MidiScriptEngine: Exec1 Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    if(m_pEngine == NULL)
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
Input:   Code string
Output:  false if an exception
-------- ------------------------------------------------------ */
bool MidiScriptEngine::internalExecute(QString scriptCode) {
    // A special version of safeExecute since we're evaluating strings, not actual functions
    //  (execute() would print an error that it's not a function every time a timer fires.)
    if(m_pEngine == NULL)
        return false;

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
    if (error!="") {
        error = QString("%1: %2 at line %3, column %4 of script code:\n%5\n")
        .arg(error)
        .arg(result.errorMessage())
        .arg(result.errorLineNumber())
        .arg(result.errorColumnNumber())
        .arg(scriptCode);

        if (m_midiDebug) qCritical() << "MidiScriptEngine:" << error;
        else scriptErrorDialog(error);
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(scriptCode);

    if (checkException())
        return false;

    // If it's not a function, we're done.
    if (!scriptFunction.isFunction())
        return true;

    // If it does happen to be a function, call it.
    scriptFunction.call(QScriptValue());
    if (checkException())
        return false;

    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, data string (e.g. device ID)
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeExecute(QString function, QString data) {
    //qDebug() << QString("MidiScriptEngine: Exec2 Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    if(m_pEngine == NULL) {
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(function);

    if (checkException())
        return false;
    if (!scriptFunction.isFunction())
        return false;

    QScriptValueList args;
    args << QScriptValue(data);

    scriptFunction.call(QScriptValue(), args);
    if (checkException())
        return false;
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, ponter to data buffer, length of buffer
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeExecute(QString function, const unsigned char data[],
                                    unsigned int length) {

    if(m_pEngine == NULL) {
        return false;
    }

    if (!m_pEngine->canEvaluate(function)) {
        qCritical() << "MidiScriptEngine: ?Syntax error in function " << function;
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(function);

    if (checkException())
        return false;
    if (!scriptFunction.isFunction())
        return false;

    // These funky conversions are required in order to
    //  get the byte array into ECMAScript complete and unharmed.
    //  Don't change this or I will hurt you -- Sean
    QVector<QChar> temp(length);
    for (unsigned int i=0; i < length; i++) {
        temp[i]=data[i];
    }
    QString buffer = QString(temp.constData(),length);
    QScriptValueList args;
    args << QScriptValue(buffer);
    args << QScriptValue(length);

    scriptFunction.call(QScriptValue(), args);
    if (checkException())
        return false;
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, channel #, control #, value, status
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeExecute(QString function, char channel,
                                   char control, char value,
                                   MidiStatusByte status,
                                   QString group) {
    //qDebug() << QString("MidiScriptEngine: Exec2 Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    if(m_pEngine == NULL) {
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(function);

    if (checkException())
        return false;
    if (!scriptFunction.isFunction())
        return false;

    QScriptValueList args;
    args << QScriptValue(channel);
    args << QScriptValue(control);
    args << QScriptValue(value);
    args << QScriptValue(status);
    args << QScriptValue(group);

    scriptFunction.call(QScriptValue(), args);
    if (checkException())
        return false;
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Check to see if a script threw an exception
   Input:   QScriptValue returned from call(scriptFunctionName)
   Output:  true if there was an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::checkException() {
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
                            .arg(line)
                            .arg((filename.isEmpty() ? "" : filename))
                            .arg(errorMessage);

        if (filename.isEmpty())
            errorText = QString(tr("Uncaught exception at line %1 in passed code: %2"))
                        .arg(line)
                        .arg(errorMessage);

        if (m_midiDebug)
            qCritical() << "MidiScriptEngine:" << errorText
                        << "\nBacktrace:\n"
                        << backtrace;
        else scriptErrorDialog(errorText);
        return true;
    }
    return false;
}

/* -------- ------------------------------------------------------
Purpose: Common error dialog creation code for run-time exceptions
            Allows users to ignore the error or reload the mappings
Input:   Detailed error string
Output:  -
-------- ------------------------------------------------------ */
void MidiScriptEngine::scriptErrorDialog(QString detailedError) {
    qWarning() << "MidiScriptEngine:" << detailedError;
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_WARNING);
    props->setTitle(tr("MIDI script error"));
    props->setText(tr("A MIDI control you just used is not working properly."));
    props->setInfoText(tr("<html>(The MIDI script code needs to be fixed.)"
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
void MidiScriptEngine::errorDialogButton(QString key, QMessageBox::StandardButton button) {

    // Something was clicked, so disable this signal now
    disconnect(ErrorDialogHandler::instance(), SIGNAL(stdButtonClicked(QString, QMessageBox::StandardButton)),
        this, SLOT(errorDialogButton(QString, QMessageBox::StandardButton)));

    if (button == QMessageBox::Retry) emit(resetController());
}

/* -------- ------------------------------------------------------
   Purpose: Returns a list of functions available in the QtScript
            code
   Input:   -
   Output:  functionList QStringList
   -------- ------------------------------------------------------ */
QStringList MidiScriptEngine::getScriptFunctions() {
    m_scriptEngineLock.lock();
    QStringList ret = m_scriptFunctions;
    m_scriptEngineLock.unlock();
    return ret;
}

void MidiScriptEngine::generateScriptFunctions(QString scriptCode) {

//     QStringList functionList;
    QStringList codeLines = scriptCode.split("\n");

//     qDebug() << "MidiScriptEngine: m_scriptCode=" << m_scriptCode;

    if (m_midiDebug)
        qDebug() << "MidiScriptEngine:" << codeLines.count() << "lines of code being searched for functions";

    // grep 'function' midi/midi-mappings-scripts.js|grep -i '(msg)'|sed -e 's/function \(.*\)(msg).*/\1/i' -e 's/[= ]//g'
    QRegExp rx("*.*function*(*)*");    // Find all lines with function names in them
    rx.setPatternSyntax(QRegExp::Wildcard);

    int position = codeLines.indexOf(rx);

    while (position != -1) {    // While there are more matches

        QString line = codeLines.takeAt(position);    // Pull & remove the current match from the list.

        if (line.indexOf('#') != 0 && line.indexOf("//") != 0) {    // ignore commented out lines
            QStringList field = line.split(" ");
            if (m_midiDebug) qDebug() << "MidiScriptEngine: Found function:" << field[0]
                                      << "at line" << position;
            m_scriptFunctions.append(field[0]);
        }
        position = codeLines.indexOf(rx);
    }

}

ControlObjectThread* MidiScriptEngine::getControlObjectThread(QString group, QString name) {

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
double MidiScriptEngine::getValue(QString group, QString name) {


    // When this function runs, assert that somebody is holding the script
    // engine lock.
    bool lock = m_scriptEngineLock.tryLock();
    Q_ASSERT(!lock);
    if(lock) {
        m_scriptEngineLock.unlock();
    }

    //qDebug() << QString("----------------------------------MidiScriptEngine: GetValue Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    ControlObjectThread *cot = getControlObjectThread(group, name);
    if (cot == NULL) {
        qWarning() << "MidiScriptEngine: Unknown control" << group << name;
        return 0.0;
    }

    return cot->get();
}

/* -------- ------------------------------------------------------
   Purpose: Sets new value of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::setValue(QString group, QString name, double newValue) {

    // When this function runs, assert that somebody is holding the script
    // engine lock.
    bool lock = m_scriptEngineLock.tryLock();
    Q_ASSERT(!lock);
    if(lock) {
        m_scriptEngineLock.unlock();
    }

    if(isnan(newValue)) {
        qWarning() << "MidiScriptEngine: script setting [" << group << "," << name
                 << "] to NotANumber, ignoring.";
        return;
    }

    //qDebug() << QString("----------------------------------MidiScriptEngine: SetValue Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    ControlObjectThread *cot = getControlObjectThread(group, name);

    if(cot != NULL && !m_st.ignore(group,name,newValue)) {
        cot->slotSet(newValue);
    }

}

/* -------- ------------------------------------------------------
   Purpose: qDebugs script output so it ends up in mixxx.log
   Input:   String to log
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::log(QString message) {

    qDebug()<<message;
}

/* -------- ------------------------------------------------------
   Purpose: Emits valueChanged() so device outputs update
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::trigger(QString group, QString name) {
    // When this function runs, assert that somebody is holding the script
    // engine lock.
    bool lock = m_scriptEngineLock.tryLock();
    Q_ASSERT(!lock);
    if(lock) {
        m_scriptEngineLock.unlock();
    }

    ControlObjectThread *cot = getControlObjectThread(group, name);
    if(cot != NULL) {
        cot->slotSet(cot->get());
    }
}

/* -------- ------------------------------------------------------
   Purpose: (Dis)connects a ControlObject valueChanged() signal to/from a script function
   Input:   Control group (e.g. [Channel1]), Key name (e.g. [filterHigh]),
                script function name, true if you want to disconnect
   Output:  true if successful
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::connectControl(QString group, QString name, QString function, bool disconnect) {
    ControlObject* cobj = ControlObject::getControl(ConfigKey(group,name));

    if (cobj == NULL) {
        qWarning() << "MidiScriptEngine: script connecting [" << group << "," << name
                   << "], which is non-existent. ignoring.";
        return false;
    }

    // Don't add duplicates
    if (!disconnect && m_connectedControls.contains(cobj->getKey(), function)) return true;

    // When this function runs, assert that somebody is holding the script
    // engine lock.
    bool lock = m_scriptEngineLock.tryLock();
    Q_ASSERT(!lock);
    if(lock) {
        m_scriptEngineLock.unlock();
    }

    //qDebug() << QString("MidiScriptEngine: Connect Thread ID=%1").arg(QThread::currentThreadId(),0,16);


    if(m_pEngine == NULL) {
        return false;
    }

    QScriptValue slot = m_pEngine->evaluate(function);

    if(!checkException() && slot.isFunction()) {
        if(disconnect) {
//             qDebug() << "MidiScriptEngine::connectControl disconnected " << group << name << " from " << function;
            m_connectedControls.remove(cobj->getKey(), function);
            // Only disconnect the signal if there are no other instances of this control using it
            if (!m_connectedControls.contains(cobj->getKey())) {
                this->disconnect(cobj, SIGNAL(valueChanged(double)),
                                this, SLOT(slotValueChanged(double)));
                this->disconnect(cobj, SIGNAL(valueChangedFromEngine(double)),
                                this, SLOT(slotValueChanged(double)));
            }
        } else {
//             qDebug() << "MidiScriptEngine::connectControl connected " << group << name << " to " << function;
            connect(cobj, SIGNAL(valueChanged(double)),
                    this, SLOT(slotValueChanged(double)),
                    Qt::QueuedConnection);
            connect(cobj, SIGNAL(valueChangedFromEngine(double)),
                    this, SLOT(slotValueChanged(double)),
                    Qt::QueuedConnection);
            m_connectedControls.insert(cobj->getKey(), function);
        }
        return true;
    }

    return false;
}

/* -------- ------------------------------------------------------
   Purpose: Receives valueChanged() slots from ControlObjects, and
   fires off the appropriate script function.
   -------- ------------------------------------------------------ */
void MidiScriptEngine::slotValueChanged(double value) {
    m_scriptEngineLock.lock();

    ControlObject* sender = (ControlObject*)this->sender();
    if(sender == NULL) {
        qWarning() << "MidiScriptEngine::slotValueChanged() Shouldn't happen -- sender == NULL";
        m_scriptEngineLock.unlock();
        return;
    }
    ConfigKey key = sender->getKey();

    //qDebug() << QString("MidiScriptEngine: slotValueChanged Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    if(m_connectedControls.contains(key)) {
        QMultiHash<ConfigKey, QString>::iterator i = m_connectedControls.find(key);
        while (i != m_connectedControls.end() && i.key() == key) {
            QString function = i.value();

//             qDebug() << "MidiScriptEngine::slotValueChanged() received signal from " << key.group << key.item << " ... firing : " << function;

            // Could branch to safeExecute from here, but for now do it this way.
            QScriptValue function_value = m_pEngine->evaluate(function);
            QScriptValueList args;
            args << QScriptValue(value);
            args << QScriptValue(key.group); // Added by Math`
            args << QScriptValue(key.item);  // Added by Math`
            QScriptValue result = function_value.call(QScriptValue(), args);
            if (result.isError()) {
                qWarning()<< "MidiScriptEngine: Call to " << function << " resulted in an error:  " << result.toString();
            }
            ++i;
        }
    } else {
        qWarning() << "MidiScriptEngine::slotValueChanged() Received signal from ControlObject that is not connected to a script function.";
    }

    m_scriptEngineLock.unlock();
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate a script file
   Input:   Script filename
   Output:  false if the script file has errors or doesn't exist
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeEvaluate(QString scriptName, QList<QString> scriptPaths) {

    if(m_pEngine == NULL) {
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
        QListIterator<QString> it(scriptPaths);
        do {
            filename = it.next()+scriptName;
            input.setFileName(filename);
        } while (it.hasNext() && !input.exists());
    }

    qDebug() << "MidiScriptEngine: Loading" << filename;

    // Read in the script file
    if (!input.open(QIODevice::ReadOnly)) {
        QString errorLog =
            QString("MidiScriptEngine: Problem opening the script file: %1, error # %2, %3")
                .arg(filename)
                .arg(input.error())
                .arg(input.errorString());

        // GUI actions do not belong in the MSE. They should be passed to
        // the above layers, along with input.errorString(), and that layer
        // can take care of notifying the user. The script engine should do
        // one thign and one thign alone -- run the scripts.
        if (m_midiDebug) {
            qCritical() << errorLog;
        } else {
            qWarning() << errorLog;
            if (m_midiPopups) {
                ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
                props->setType(DLG_WARNING);
                props->setTitle("MIDI script file problem");
                props->setText(QString("There was a problem opening the MIDI script file %1.").arg(filename));
                props->setInfoText(input.errorString());

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
    if (error!="") {
        error = QString("%1 at line %2, column %3 in file %4: %5")
                        .arg(error)
                        .arg(result.errorLineNumber())
                        .arg(result.errorColumnNumber())
                        .arg(filename)
                        .arg(result.errorMessage());

        if (m_midiDebug) qCritical() << "MidiScriptEngine:" << error;
        else {
            qWarning() << "MidiScriptEngine:" << error;
            if (m_midiPopups) {
                ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
                props->setType(DLG_WARNING);
                props->setTitle("MIDI script file error");
                props->setText(QString("There was an error in the MIDI script file %1.").arg(filename));
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
    if(checkException())
        return false;

    // Add the code we evaluated to our index
    generateScriptFunctions(scriptCode);

    return true;
}

/*
 * Check whether a source file that was evaluated()'d has errors.
 */
bool MidiScriptEngine::hasErrors(QString filename) {
    m_scriptEngineLock.lock();
    bool ret = m_scriptErrors.contains(filename);
    m_scriptEngineLock.unlock();
    return ret;
}

/*
 * Get the errors for a source file that was evaluated()'d
 */
const QStringList MidiScriptEngine::getErrors(QString filename) {
    QStringList ret;
    m_scriptEngineLock.lock();
    if(m_scriptErrors.contains(filename))
        ret = m_scriptErrors.value(filename);
    m_scriptEngineLock.unlock();
    return ret;
}


/* -------- ------------------------------------------------------
   Purpose: Creates & starts a timer that runs some script code
                on timeout
   Input:   Number of milliseconds, script function to call,
                whether it should fire just once
   Output:  The timer's ID, 0 if starting it failed
   -------- ------------------------------------------------------ */
int MidiScriptEngine::beginTimer(int interval, QString scriptCode, bool oneShot) {
    // When this function runs, assert that somebody is holding the script
    // engine lock.
    bool lock = m_scriptEngineLock.tryLock();
    Q_ASSERT(!lock);
    if(lock) {
        m_scriptEngineLock.unlock();
    }

    if (interval<20) {
        qWarning() << "Timer request for" << interval << "ms is too short. Setting to the minimum of 20ms.";
        interval=20;
    }
    // This makes use of every QObject's internal timer mechanism. Nice, clean, and simple.
    // See http://doc.trolltech.com/4.6/qobject.html#startTimer for details
    int timerId = startTimer(interval);
    QPair<QString, bool> timerTarget;
    timerTarget.first = scriptCode;
    timerTarget.second = oneShot;
    m_timers[timerId]=timerTarget;
    if (timerId==0) qWarning() << "MIDI Script timer could not be created";
    else if (m_midiDebug) {
        if (oneShot) qDebug() << "Starting one-shot timer:" << timerId;
        else qDebug() << "Starting timer:" << timerId;
    }
    return timerId;
}

/* -------- ------------------------------------------------------
   Purpose: Stops & removes a timer
   Input:   ID of timer to stop
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::stopTimer(int timerId) {
    // When this function runs, assert that somebody is holding the script
    // engine lock.
    bool lock = m_scriptEngineLock.tryLock();
    Q_ASSERT(!lock);
    if(lock) m_scriptEngineLock.unlock();

    if (!m_timers.contains(timerId)) {
        qWarning() << "Killing timer" << timerId << ": That timer does not exist!";
        return;
    }
    if (m_midiDebug) qDebug() << "Killing timer:" << timerId;

    killTimer(timerId);
    m_timers.remove(timerId);
}

/* -------- ------------------------------------------------------
   Purpose: Stops & removes all timers (for shutdown)
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::stopAllTimers() {
    // When this function runs, assert that somebody is holding the script
    // engine lock.
    bool lock = m_scriptEngineLock.tryLock();
    Q_ASSERT(!lock);
    if(lock) m_scriptEngineLock.unlock();

    QMutableHashIterator<int, QPair<QString, bool> > i(m_timers);
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
void MidiScriptEngine::timerEvent(QTimerEvent *event) {
    int timerId = event->timerId();

    m_scriptEngineLock.lock();

    // See if this is a scratching timer
    if (m_scratchTimers.contains(timerId)) {
        m_scriptEngineLock.unlock();
        scratchProcess(timerId);
        return;
    }

    if (!m_timers.contains(timerId)) {
        qWarning() << "Timer" << timerId << "fired but there's no function mapped to it!";
        m_scriptEngineLock.unlock();
        return;
    }

    QPair<QString, bool> timerTarget = m_timers[timerId];
    if (timerTarget.second) stopTimer(timerId);

    internalExecute(timerTarget.first);
    m_scriptEngineLock.unlock();
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
void MidiScriptEngine::scratchEnable(int deck, int intervalsPerRev, float rpm, float alpha, float beta) {

    // If we're already scratching this deck, override that with this request
    if (m_dx[deck]) {
//         qDebug() << "Already scratching deck" << deck << ". Overriding.";
        int timerId = m_scratchTimers.key(deck);
        killTimer(timerId);
        m_scratchTimers.remove(timerId);
    }

    // Controller resolution in intervals per second at normal speed (rev/min * ints/rev * mins/sec)
    float intervalsPerSecond = (rpm * intervalsPerRev)/60;

    m_dx[deck] = 1/intervalsPerSecond;
    m_intervalAccumulator[deck] = 0;
    m_ramp[deck] = false;

    QString group = QString("[Channel%1]").arg(deck);

    // Ramp
    float initVelocity = 0.0;   // Default to stopped

    // See if the deck is already being scratched
    ControlObjectThread *cot = getControlObjectThread(group, "scratch2_enable");
    if (cot != NULL && cot->get() == 1) {
        // If so, set the filter's initial velocity to the scratch speed
        cot = getControlObjectThread(group, "scratch2");
        if (cot != NULL) initVelocity=cot->get();
    }
    else {
        // See if deck is playing
        cot = getControlObjectThread(group, "play");
        if (cot != NULL && cot->get() == 1) {
            // If so, set the filter's initial velocity to the playback speed
            float rate=0;
            cot = getControlObjectThread(group, "rate");
            if (cot != NULL) rate = cot->get();
            cot = getControlObjectThread(group, "rateRange");
            if (cot != NULL) rate = rate * cot->get();
            // Add 1 since the deck is playing
            rate++;
            // See if we're in reverse play
            cot = getControlObjectThread(group, "reverse");
            if (cot != NULL && cot->get() == 1) rate = -rate;

            initVelocity = rate;
        }
    }

    // Initialize pitch filter (0.001s = 1ms)
    //  (We're assuming the OS actually gives us a 1ms timer below)
    if (alpha && beta) m_pitchFilter[deck]->init(0.001, initVelocity, alpha, beta);
    else m_pitchFilter[deck]->init(0.001, initVelocity); // Use filter's defaults if not specified

    int timerId = startTimer(1);    // 1ms is shortest possible, OS dependent
    // Associate this virtual deck with this timer for later processing
    m_scratchTimers[timerId] = deck;

    // Set scratch2_enable
    cot = getControlObjectThread(group, "scratch2_enable");
    if(cot != NULL) cot->slotSet(1);
}

/* -------- ------------------------------------------------------
    Purpose: Accumulates "ticks" of the controller wheel
    Input:   Virtual deck to scratch, interval value (usually +1 or -1)
    Output:  -
    -------- ------------------------------------------------------ */
void MidiScriptEngine::scratchTick(int deck, int interval) {
    m_intervalAccumulator[deck] += interval;
}

/* -------- ------------------------------------------------------
    Purpose: Applies the accumulated movement to the track speed
    Input:   ID of timer for this deck
    Output:  -
    -------- ------------------------------------------------------ */
void MidiScriptEngine::scratchProcess(int timerId) {

    int deck = m_scratchTimers[timerId];
    PitchFilter* filter = m_pitchFilter[deck];
    QString group = QString("[Channel%1]").arg(deck);

    if (!filter) {
        qWarning() << "Scratch filter pointer is null on deck" << deck;
        return;
    }

    // Give the filter a data point:

    // If we're ramping to end scratching, feed fixed data
    if (m_ramp[deck]) filter->observation(m_rampTo[deck]*0.001);
    //  This will (and should) be 0 if no net ticks have been accumulated (i.e. the wheel is stopped)
    else filter->observation(m_dx[deck] * m_intervalAccumulator[deck]);

    // Actually do the scratching
    ControlObjectThread *cot = getControlObjectThread(group, "scratch2");
    if(cot != NULL) cot->slotSet(filter->currentPitch());

    // Reset accumulator
    m_intervalAccumulator[deck] = 0;

    // If we're ramping and the current pitch is really close to the rampTo value,
    //  end scratching
//     if (m_ramp[deck]) qDebug() << "Ramping to" << m_rampTo[deck] << " Currently at:" << filter->currentPitch();
    if (m_ramp[deck] && fabs(m_rampTo[deck]-filter->currentPitch()) <= 0.00001) {

        m_ramp[deck] = false;   // Not ramping no mo'

        // Clear scratch2_enable
        cot = getControlObjectThread(group, "scratch2_enable");
        if(cot != NULL) cot->slotSet(0);

        // Remove timer
        killTimer(timerId);
        m_scratchTimers.remove(timerId);

        m_dx[deck] = 0;
    }
}

/* -------- ------------------------------------------------------
    Purpose: Stops scratching the specified virtual deck
    Input:   Virtual deck to stop scratching
    Output:  -
    -------- ------------------------------------------------------ */
void MidiScriptEngine::scratchDisable(int deck) {

    QString group = QString("[Channel%1]").arg(deck);

    // See if deck is playing
    ControlObjectThread *cot = getControlObjectThread(group, "play");
    if (cot != NULL && cot->get() == 1) {
        // If so, set the target velocity to the playback speed
        float rate=0;
        // Get the pitch slider value
        cot = getControlObjectThread(group, "rate");
        if (cot != NULL) rate = cot->get();
        // Multiply by the pitch range
        cot = getControlObjectThread(group, "rateRange");
        if (cot != NULL) rate = rate * cot->get();
        // Add 1 since the deck is playing
        rate++;
        // See if we're in reverse play
        cot = getControlObjectThread(group, "reverse");
        if (cot != NULL && cot->get() == 1) rate = -rate;

        m_rampTo[deck] = rate;
    }
    else m_rampTo[deck]=0.0;

    m_ramp[deck] = true;    // Activate the ramping in scratchProcess()
}

/*  -------- ------------------------------------------------------
    Purpose: [En/dis]ables soft-takeover status for a particular MixxxControl
    Input:   MixxxControl group and key values,
                whether to set the soft-takeover status or not
    Output:  -
    -------- ------------------------------------------------------ */
void MidiScriptEngine::softTakeover(QString group, QString name, bool set) {
    MixxxControl mc = MixxxControl(group,name);
    if (set) m_st.enable(mc);
    else m_st.disable(mc);
}
