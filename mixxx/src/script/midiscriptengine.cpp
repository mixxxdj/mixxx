/***************************************************************************
                          midiscriptengine.cpp  -  description
                          -------------------
    begin                : Fri Dec 12 2008
    copyright            : (C) 2008 by Sean M. Pappalardo
                                       "Holy crap, I wrote new code!"
    email                : pegasus@renegadetech.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qapplication.h>

#include "midiscriptengine.h"
#include "../controlobject.h"
#include "../controlobjectthread.h"

MidiScriptEngine::MidiScriptEngine(MidiObject* midi_object) :
    m_pEngine(NULL)
{
    m_pMidiObject = midi_object;
    // rryan 1/30 commented -- now started after the object is moveToThread'd
    // Start the thread.
    //start();
    qRegisterMetaType<MidiCategory>("MidiCategory");
    
    connect(this, SIGNAL(sigEvaluate(QString)),
            this, SLOT(safeEvaluate(QString)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(sigExecute(QString)),
            this, SLOT(safeExecute(QString)),
            Qt::BlockingQueuedConnection);
    
    connect(this, SIGNAL(sigExecute(QString,char,QString,char,char,MidiCategory)),
            this, SLOT(safeExecute(QString,char,QString,char,char,MidiCategory)),
            Qt::BlockingQueuedConnection);
}

MidiScriptEngine::~MidiScriptEngine() {

    // Stop processing the event loop and terminate the thread.
    quit();
    
    // Clear the m_connectedControls hash so we stop responding
    // to signals.
    m_connectedControls.clear();

    // Wait for the thread to terminate
    wait();

    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if(m_pEngine != NULL) {
        QScriptEngine *engine = m_pEngine;
        m_pEngine = NULL;
        engine->deleteLater();
    }
    
}

bool MidiScriptEngine::isReady() {
    return m_pEngine != NULL;
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

    //qDebug() << QString("----------------------------------MidiScriptEngine: Run Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    // Create the MidiScriptEngine
    m_pEngine = new QScriptEngine(this);

//     qDebug() << "MidiScriptEngine::run() m_pEngine->parent() is " << m_pEngine->parent();
//     qDebug() << "MidiScriptEngine::run() m_pEngine->thread() is " << m_pEngine->thread();

    // Make this MidiScriptEngine instance available to scripts as
    // 'engine'.
    QScriptValue engineGlobalObject = m_pEngine->globalObject();
    engineGlobalObject.setProperty("engine", m_pEngine->newQObject(this));
    engineGlobalObject.setProperty("midi", m_pEngine->newQObject(m_pMidiObject));

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
    emit(sigEvaluate(filepath));
    if(hasErrors(filepath))
        return false;
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::execute(QString function) {
    emit(sigExecute(function));
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, channel #, device name, control #, value, category
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::execute(QString function, char channel, QString device, char control, char value,  MidiCategory category) {
    emit(sigExecute(function,channel,
                    device,control,
                    value,category));
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeExecute(QString function) {
    //qDebug() << QString("MidiScriptEngine: Exec1 Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    if(m_pEngine == NULL) {
        return false;
    }
    
    if (!m_pEngine->canEvaluate(function)) {
        qCritical() << "MidiScriptEngine: ?Syntax error in function " << function;
        return false;
    }
    
    QScriptValue scriptFunction = m_pEngine->evaluate(function);
    
    if (checkException()) return false;
    if (!scriptFunction.isFunction()) return false;

    scriptFunction.call(QScriptValue());
    if (checkException()) return false;
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, channel #, device name, control #, value, category
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeExecute(QString function, char channel,
                                   QString device, char control,
                                   char value,  MidiCategory category) {
    //qDebug() << QString("MidiScriptEngine: Exec2 Thread ID=%1").arg(QThread::currentThreadId(),0,16);

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

    QScriptValueList args;
    args << QScriptValue(m_pEngine, channel);
    args << QScriptValue(m_pEngine, device);
    args << QScriptValue(m_pEngine, control);
    args << QScriptValue(m_pEngine, value);
    args << QScriptValue(m_pEngine, category);

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
        error << filename << errorMessage << QString(line);
        m_scriptErrors.insert(filename, error);
        
        qCritical() << "MidiScriptEngine uncaught exception : "
                    << errorMessage
                    << "at " << filename << " line"
                    << line;
        // qCritical() << "MidiScriptEngine: uncaught exception"
        //             << m_pEngine->uncaughtException().toString()
        //             << "\nBacktrace:\n"
        //             << m_pEngine->uncaughtExceptionBacktrace();
        
        return true;
    }
    return false;
}


/* -------- ------------------------------------------------------
   Purpose: Returns a list of functions available in the QtScript
            code
   Input:   -
   Output:  functionList QStringList
   -------- ------------------------------------------------------ */
QStringList MidiScriptEngine::getScriptFunctions() {
    return m_scriptFunctions;
}

void MidiScriptEngine::generateScriptFunctions(QString scriptCode) { 

//     QStringList functionList;
    QStringList codeLines = scriptCode.split("\n");

//     qDebug() << "MidiScriptEngine: m_scriptCode=" << m_scriptCode;

    qDebug() << "MidiScriptEngine:" << codeLines.count() << "lines of code being searched for functions";

    // grep 'function' midi/midi-mappings-scripts.js|grep -i '(msg)'|sed -e 's/function \(.*\)(msg).*/\1/i' -e 's/[= ]//g'
    QRegExp rx("*.*function*(*)*");    // Find all lines with function names in them
    rx.setPatternSyntax(QRegExp::Wildcard);

    int position = codeLines.indexOf(rx);

    while (position != -1) {    // While there are more matches

        QString line = codeLines.takeAt(position);    // Pull & remove the current match from the list.

        if (line.indexOf('#') != 0 && line.indexOf("//") != 0) {    // ignore commented out lines
            QStringList field = line.split(" ");
            qDebug() << "MidiScriptEngine: Found function:" << field[0] << "at line" << position;
//             functionList.append(field[0]);
            m_scriptFunctions.append(field[0]);
        }
        position = codeLines.indexOf(rx);
    }

//     m_scriptFunctions = functionList;
}

/* -------- ------------------------------------------------------
   Purpose: Returns the current value of a Mixxx control (for scripts)
   Input:   Control group (e.g. [Channel1]), Key name (e.g. [filterHigh])
   Output:  The value
   -------- ------------------------------------------------------ */
double MidiScriptEngine::getValue(QString group, QString name) {
    //qDebug() << QString("----------------------------------MidiScriptEngine: GetValue Thread ID=%1").arg(QThread::currentThreadId(),0,16);
    
//     ControlObject *pot = ControlObject::getControl(ConfigKey(group, name));
    ControlObjectThread *cot = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, name)));
    if (cot == NULL) {
        qDebug() << "MidiScriptEngine: Unknown control" << group << name;
        return 0.0;
    }
//     return pot->get();
    double temp = cot->get();
    delete cot;
    return temp;
}

/* -------- ------------------------------------------------------
   Purpose: Sets new value of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::setValue(QString group, QString name, double newValue) {
//     ControlObject *pot = ControlObject::getControl(ConfigKey(group, name));
//     pot->queueFromThread(newValue);
    //qDebug() << QString("----------------------------------MidiScriptEngine: SetValue Thread ID=%1").arg(QThread::currentThreadId(),0,16);
    ControlObjectThread *cot = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, name)));
    cot->slotSet(newValue);
    delete cot;
}

/* -------- ------------------------------------------------------
   Purpose: Emits valueChanged() so device outputs update
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::trigger(QString group, QString name) {
    ControlObjectThread *cot = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, name)));
    cot->slotSet(cot->get());
    delete cot;
}

/* -------- ------------------------------------------------------
   Purpose: (Dis)connects a ControlObject valueChanged() signal to/from a script function
   Input:   Control group (e.g. [Channel1]), Key name (e.g. [filterHigh]),
                script function name, true if you want to disconnect
   Output:  true if successful
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::connectControl(QString group, QString name, QString function, bool disconnect) {
    ControlObject* cobj = ControlObject::getControl(ConfigKey(group,name));
    
    //qDebug() << QString("MidiScriptEngine: Connect Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    if(m_pEngine == NULL) {
        return false;
    }

    if (!m_pEngine->canEvaluate(function)) {
        qCritical() << "MidiScriptEngine: ?Syntax error in function " << function;
        return false;
    }
    QScriptValue slot = m_pEngine->evaluate(function);


    if(!checkException() && slot.isFunction()) {
        if(disconnect) {
//             qDebug() << "MidiScriptEngine::connectControl disconnected " << group << name << " from " << function;
            this->disconnect(cobj, SIGNAL(valueChanged(double)), this, SLOT(slotValueChanged(double)));
            this->disconnect(cobj, SIGNAL(valueChangedFromEngine(double)), this, SLOT(slotValueChanged(double)));
            m_connectedControls.remove(cobj->getKey());
        } else {
//             qDebug() << "MidiScriptEngine::connectControl connected " << group << name << " to " << function;
            connect(cobj, SIGNAL(valueChanged(double)), this, SLOT(slotValueChanged(double)));
            connect(cobj, SIGNAL(valueChangedFromEngine(double)), this, SLOT(slotValueChanged(double)));
            m_connectedControls.insert(cobj->getKey(), function);
        }
        return true;
    }

    return false;
    // The following code is not used now that we emulate qScriptConnect.
    /*
    if (!checkException() && slot.isFunction()) {    // If no problems,
        // Do the deed
        if (disconnect) {
            //Get the saved ControlObjectThread pointer that we created when we connected the slot/signal.
//             ControlObjectThread* cobj = m_controlCache.take(ConfigKey(group,name));
            qScriptDisconnect(cobj, SIGNAL(valueChangedFromEngine(double)), QScriptValue(), slot);  // Needed for rate_temp*
            if (qScriptDisconnect(cobj, SIGNAL(valueChanged(double)), QScriptValue(), slot))
                qDebug() << "MidiScriptEngine:" << group << name << "valueChanged() disconnected from" << function;
            else qDebug() << "MidiScriptEngine:" << group << name << "valueChanged() disconnect from" << function << "FAILED!";
            
//             delete cobj;
        }
        else {
//             ControlObjectThread* cobj = new ControlObjectThread(ControlObject::getControl(ConfigKey(group,name)));
            
            qScriptConnect(cobj, SIGNAL(valueChangedFromEngine(double)), QScriptValue(), slot); // Needed for rate_temp*
            if (qScriptConnect(cobj, SIGNAL(valueChanged(double)), QScriptValue(), slot))
                qDebug() << "MidiScriptEngine:" << group << name << "valueChanged() connected to" << function;
            else qDebug() << "MidiScriptEngine:" << group << name << "valueChanged() connect to" << function << "FAILED!";
            
            //Save the ControlObjectThread pointer so we can disconnect this properly later.
//             m_controlCache.insert(ConfigKey(group,name), cobj);
        }
        
        return true;
        
    } else {
        qWarning() << "MidiScriptEngine:" << group << name << "didn't connect/disconnect to/from" << function;
        return false;
        }
    */
}

/* -------- ------------------------------------------------------
   Purpose: Receives valueChanged() slots from ControlObjects, and
   fires off the appropriate script function.
   -------- ------------------------------------------------------ */
void MidiScriptEngine::slotValueChanged(double value) {
    ControlObject* sender = (ControlObject*)this->sender();
    if(sender == NULL) {
        qDebug() << "MidiScriptEngine::slotValueChanged() Shouldn't happen -- sender == NULL";
        return;
    }
    ConfigKey key = sender->getKey();

    //qDebug() << QString("MidiScriptEngine: slotValueChanged Thread ID=%1").arg(QThread::currentThreadId(),0,16);

    if(m_connectedControls.contains(key)) {
        QString function = m_connectedControls.value(key);
//         qDebug() << "MidiScriptEngine::slotValueChanged() received signal from " << key.group << key.item << " ... firing : " << function;

        // Could branch to safeExecute from here, but for now do it this way.
        QScriptValue function_value = m_pEngine->evaluate(function);
        QScriptValueList args;
        args << QScriptValue(m_pEngine, value);
        function_value.call(QScriptValue(), args);
        
    } else {
        qDebug() << "MidiScriptEngine::slotValueChanged() Received signal from ControlObject that is not connected to a script function.";
    }
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate a script file
   Input:   Script filename
   Output:  false if the script file has errors or doesn't exist
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeEvaluate(QString filename) {
    if(!isReady()) {
        return false;
    }

    // Read in the script file
    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly)) {
        qCritical() << "MidiScriptEngine: Problem opening the script file: "
                    << filename
                    << ", error #"
                    << input.error();
        return false;
    }
    QString scriptCode = "";
    scriptCode.append(input.readAll());
    scriptCode.append('\n');
    input.close();
    
    if (!m_pEngine->canEvaluate(scriptCode)) {
        qCritical() << "MidiScriptEngine: ?Syntax error in script file:" << filename;
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
    // TODO(rryan) add locking!
    if(m_scriptErrors.contains(filename))
        return true;
    else return false;
}

/*
 * Get the errors for a source file that was evaluated()'d
 */ 
const QStringList MidiScriptEngine::getErrors(QString filename) {
    // TODO(rryan) add locking!
    if(m_scriptErrors.contains(filename))
        return m_scriptErrors.value(filename);
}


