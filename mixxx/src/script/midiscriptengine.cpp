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

#include <QMutexLocker>

// QMutex MidiScriptEngine::m_mutex;

ExecuteEvent::ExecuteEvent(QString function) :
    QEvent(EXECUTE_EVENT_TYPE),
    m_function(function),
    m_bSimple(true)
{
}
    
ExecuteEvent::ExecuteEvent(QString function,
                           char channel,
                           QString device,
                           char control,
                           char value,
                           MidiCategory category) :
    QEvent(EXECUTE_EVENT_TYPE),
    m_bSimple(false),
    m_function(function),
    m_channel(channel),
    m_device(device),
    m_control(control),
    m_value(value),
    m_category(category)
{
}



/* -------- ------------------------------------------------------
   Purpose: Open script file, read into QString
   Input:   Path to script file
   Output:  -
   -------- ------------------------------------------------------ */
MidiScriptEngine::MidiScriptEngine() :
    m_pEngine(NULL)
{
    // rryan 1/30 commented -- now started after the object is moveToThread'd
    // Start the thread.
    //start();
}

MidiScriptEngine::~MidiScriptEngine() {

    // Stop processing the event loop and terminate the thread.
    quit();
    
    // Clear the m_connectedControls hash so we stop responding
    // to signals.
    m_connectedControls.clear();

    // Delete the script engine, first clearing the pointer so that
    // other threads will not get the dead pointer after we delete it.
    if(m_pEngine != NULL) {
        QScriptEngine *engine = m_pEngine;
        m_pEngine = NULL;
        delete engine;
    }
    
}

bool MidiScriptEngine::event(QEvent* e) {
  //qDebug() << QString("----------------------------------MidiScriptEngine: Event Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);
    qDebug() << "MidiScriptEngine::event() Event Type:" << e->type();
    if(e->type() == EXECUTE_EVENT_TYPE) {
        ExecuteEvent *ee = (ExecuteEvent*)e;
        if(ee->isSimple()) {
            safeExecute(ee->function());
        } else {
            safeExecute(ee->function(),
                        ee->channel(),
                        ee->device(),
                        ee->control(),
                        ee->value(),
                        ee->category());
        }
        return true;        
    }
    // If it's not an ExecuteEvent, process it normally
    return QObject::event(e);
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

  //qDebug() << QString("----------------------------------MidiScriptEngine: Run Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);

    // Create the MidiScriptEngine
    m_pEngine = new QScriptEngine();

    qDebug() << "MidiScriptEngine::run() m_pEngine->parent() is " << m_pEngine->parent();
    qDebug() << "MidiScriptEngine::run() m_pEngine->thread() is " << m_pEngine->thread();

    // Make this MidiScriptEngine instance available to scripts as
    // 'engine'.
    engineGlobalObject = m_pEngine->globalObject();
    engineGlobalObject.setProperty("engine", m_pEngine->newQObject(this));
    
    // Run the Qt event loop indefinitely 
    exec();
}

/* -------- ------------------------------------------------------
   Purpose: Load a script file into this object and add to existing code.
   Input:   Path to script file
   Output:  true if load successful
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::loadScript(QString filepath) {

    m_lastFilepath=filepath;

    // Read in the script file
    QFile input(m_lastFilepath);
    if (!input.open(QIODevice::ReadOnly)) {
        qCritical() << "MidiScriptEngine: Problem opening the script file: " << m_lastFilepath << ", error #" << input.error();
        return false;
    }
    m_scriptCode.append(input.readAll());
    m_scriptCode.append('\n');
    input.close();
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Return the file path of the most recently loaded script
   Input:   -
   Output:  m_lastFilepath QString
   -------- ------------------------------------------------------ */
QString MidiScriptEngine::getLastFilepath() {
    return m_lastFilepath;
}

/* -------- ------------------------------------------------------
   Purpose: Clear out the script code (to load new)
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::clearCode() {
    m_scriptCode.clear();
    m_scriptFunction = QScriptValue();
    return;
}

/* -------- ------------------------------------------------------
   Purpose: Validate script syntax, then evaluate() it so the
            functions are registered & available for use.
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::evaluateScript() {
//     QMutexLocker locker(&m_mutex);  //Prevent more than one thread using the ScriptEngine at a time
//    qDebug() << QString("MidiScriptEngine: AllEval Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);
    
    if(m_pEngine == NULL) {
        return;
    }
    
    if (!m_pEngine->canEvaluate(m_scriptCode)) {
        qCritical() << "MidiScriptEngine: ?Syntax error in script file:" << m_lastFilepath;
        m_scriptCode.clear();    // Free up now-unneeded memory
        m_scriptGood=false;
        return;
    }
    m_scriptFunction = m_pEngine->evaluate(m_scriptCode);
//     if (!checkException())
//         qDebug() << "MidiScriptEngine: Script code evaluated successfully.";
    m_scriptGood=true;
    return;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::execute(QString function) {
    ExecuteEvent *ee = new ExecuteEvent(function);
    QApplication::postEvent(this, ee);
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, channel #, device name, control #, value, category
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::execute(QString function, char channel, QString device, char control, char value,  MidiCategory category) {
    ExecuteEvent *ee = new ExecuteEvent(function,
                                        channel,
                                        device,
                                        control,
                                        value,
                                        category);
    QApplication::postEvent(this, ee);
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeExecute(QString function) {
//     QMutexLocker locker(&m_mutex);  //Prevent more than one thread using the ScriptEngine at a time
  // qDebug() << QString("MidiScriptEngine: Exec1 Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);

    if(m_pEngine == NULL) {
        return false;
    }
    
    if (!m_pEngine->canEvaluate(function)) {
        qCritical() << "MidiScriptEngine: ?Syntax error in function " << function;
        return false;
    }
    m_scriptFunction = m_pEngine->evaluate(function);
    
    if (checkException()) return false;
    if (!m_scriptFunction.isFunction()) return false;

    m_scriptFunction.call(QScriptValue());
    if (checkException()) return false;
    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, channel #, device name, control #, value, category
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::safeExecute(QString function, char channel, QString device, char control, char value,  MidiCategory category) {
//     QMutexLocker locker(&m_mutex);  //Prevent more than one thread using the ScriptEngine at a time
//     qDebug() << QString("MidiScriptEngine: Exec2 Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);

    if(m_pEngine == NULL) {
        return false;
    }
    
    if (!m_pEngine->canEvaluate(function)) {
        qCritical() << "MidiScriptEngine: ?Syntax error in function " << function;
        return false;
    }
    
    m_scriptFunction = m_pEngine->evaluate(function);
    
    if (checkException()) return false;
    if (!m_scriptFunction.isFunction()) return false;

    QScriptValueList args;
    args << QScriptValue(m_pEngine, channel);
    args << QScriptValue(m_pEngine, device);
    args << QScriptValue(m_pEngine, control);
    args << QScriptValue(m_pEngine, value);
    args << QScriptValue(m_pEngine, category);

    m_scriptFunction.call(QScriptValue(), args);
    if (checkException()) return false;
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
        int line = m_pEngine->uncaughtExceptionLineNumber();
//         qCritical() << "MidiScriptEngine: uncaught exception" << m_pEngine->uncaughtException().toString() << "\nBacktrace:\n" << m_pEngine->uncaughtExceptionBacktrace();
        qCritical() << "MidiScriptEngine: uncaught exception" << m_pEngine->uncaughtException().toString() << "at line" << line;
        return true;
    }
    return false;
}

/* -------- ------------------------------------------------------
   Purpose: Return the result of the last operation
   Input:   -
   Output:  m_scriptFunction QString
   -------- ------------------------------------------------------ */
QString MidiScriptEngine::getResult() {
    return m_scriptFunction.toString();
}

/* -------- ------------------------------------------------------
   Purpose: Return if the script evaulated successfully or not
   Input:   -
   Output:  m_scriptGood boolean
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::isGood() {
    return m_scriptGood;
}

/* -------- ------------------------------------------------------
   Purpose: Return if the MidiScriptEngine is ready for use.
   Input:   -
   Output:  m_pEngine != NULL
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::isReady() {
    return m_pEngine != NULL;
}

/* -------- ------------------------------------------------------
   Purpose: Return a pointer to the script engine for Global Object access
   Input:   -
   Output:  m_pEngine QScriptEngine
   -------- ------------------------------------------------------ */
QScriptEngine * MidiScriptEngine::getEngine() {
    return m_pEngine;
}

/* -------- ------------------------------------------------------
   Purpose: Returns a list of functions available in the QtScript
            code
   Input:   -
   Output:  functionList QStringList
   -------- ------------------------------------------------------ */
QStringList MidiScriptEngine::getFunctionList() {

    QStringList functionList;
    QStringList codeLines=m_scriptCode.split("\n");

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
            functionList.append(field[0]);
        }
        position = codeLines.indexOf(rx);
    }

    return functionList;
}

/* -------- ------------------------------------------------------
   Purpose: Returns the current value of a Mixxx control (for scripts)
   Input:   Control group (e.g. [Channel1]), Key name (e.g. [filterHigh])
   Output:  The value
   -------- ------------------------------------------------------ */
double MidiScriptEngine::getValue(QString group, QString name) {
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
//     ControlObject *pot = ControlObject::getControl(ConfigKey(group, name));
//     pot->sync();    // This doesn't work
//     pot->update();   // This sometimes segfaults
//     qDebug() << "MidiScriptEngine: Running in thread" << this->thread();
    ControlObjectThread *cot = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, name)));
//     qDebug() << "MidiScriptEngine: cot in thread" << cot->thread();
//     cot->emitValueChanged();    // This doesn't work.
    cot->slotSet(cot->get());   // This sometimes segfaults
//     cot->setExtern(cot->get()); // This doesn't work
//     cot->update();  // This doesn't work either.
//     qDebug() << "MidiScriptEngine: Made it past SlotSet";
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
    
//     qDebug() << QString("MidiScriptEngine: Connect: ControlObject Thread ID=%1").arg(cobj->thread()->currentThreadId(),0,16);
    
//     QMutexLocker locker(&m_mutex);  // Prevent more than one thread using the ScriptEngine at a time
    //   qDebug() << QString("MidiScriptEngine: Connect Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);

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
            qDebug() << "MidiScriptEngine::connectControl disconnected " << group << name << " from " << function;
            this->disconnect(cobj, SIGNAL(valueChanged(double)), this, SLOT(slotValueChanged(double)));
            this->disconnect(cobj, SIGNAL(valueChangedFromEngine(double)), this, SLOT(slotValueChanged(double)));
            m_connectedControls.remove(cobj->getKey());
        } else {
            qDebug() << "MidiScriptEngine::connectControl connected " << group << name << " to " << function;
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

    //qDebug() << QString("MidiScriptEngine: slotValueChanged Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);

    if(m_connectedControls.contains(key)) {
        QString function = m_connectedControls.value(key);
        qDebug() << "MidiScriptEngine::slotValueChanged() received signal from " << key.group << key.item << " ... firing : " << function;

        // Could branch to safeExecute from here, but for now do it this way.
        QScriptValue function_value = m_pEngine->evaluate(function);
        QScriptValueList args;
        args << QScriptValue(m_pEngine, value);
        function_value.call(QScriptValue(), args);
        
    } else {
        qDebug() << "MidiScriptEngine::slotValueChanged() Received signal from ControlObject that is not connected to a script function.";
    }
}
