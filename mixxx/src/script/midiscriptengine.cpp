/***************************************************************************
                          midiscriptengine.h  -  description
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

#include "midiscriptengine.h"
#include "../controlobject.h"

/* -------- ------------------------------------------------------
   Purpose: Open script file, read into QString
   Input:   Path to script file
   Output:  -
   -------- ------------------------------------------------------ */
MidiScriptEngine::MidiScriptEngine() : m_engine() {

    engineGlobalObject = m_engine.globalObject();
    engineGlobalObject.setProperty("engine", m_engine.newQObject(this));
//     QObject *someObject = new MidiObject;
//     QScriptValue objectValue = m_engine.newQObject(someObject);
//     engineGlobalObject.setProperty("midi", objectValue);
}

MidiScriptEngine::~MidiScriptEngine() {
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
        qWarning() << "MidiScriptEngine: Problem opening the script file: " << m_lastFilepath << ", error #" << input.error();
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
    return;
}

/* -------- ------------------------------------------------------
   Purpose: Validate script syntax, then evaluate() it so the
            functions are registered & available for use.
   Input:   -
   Output:  m_result QScriptValue
   -------- ------------------------------------------------------ */
void MidiScriptEngine::evaluateScript() {
    if (!m_engine.canEvaluate(m_scriptCode)) {
        qWarning() << "MidiScriptEngine: ?Syntax error in script file:" << m_lastFilepath;
        m_scriptCode.clear();    // Free up now-unneeded memory
        m_scriptGood=false;
        return;
    }
    m_result = m_engine.evaluate(m_scriptCode);
//     if (!checkException())
//         qDebug() << "MidiScriptEngine: Script code evaluated successfully.";
    m_scriptGood=true;
    return;
}

/* -------- ------------------------------------------------------
   Purpose: Execute a script function
   Input:   -
   Output:  m_result QScriptValue
   -------- ------------------------------------------------------ */
QScriptValue MidiScriptEngine::execute(QString function) {
    if (!m_engine.canEvaluate(function)) {
        qWarning() << "MidiScriptEngine: ?Syntax error in function " << function;
        return QScriptValue();
    }
    m_result = m_engine.evaluate(function);
//     if (!checkException(m_result))
//         qDebug() << "MidiScriptEngine: Script function" << function << "executed successfully.";
    return m_result;
}

/* -------- ------------------------------------------------------
   Purpose: Check to see if a script threw an exception
   Input:   QScriptValue returned from call(scriptFunctionName)
   Output:  true if there was an exception
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::checkException() {
    if (m_engine.hasUncaughtException()) {
        int line = m_engine.uncaughtExceptionLineNumber();
//         qDebug() << "MidiScriptEngine: uncaught exception" << m_engine.uncaughtException().toString() << "\nBacktrace:\n" << m_engine.uncaughtExceptionBacktrace();
        qDebug() << "MidiScriptEngine: uncaught exception" << m_engine.uncaughtException().toString() << "at line" << line;
        return true;
    }
    return false;
}

/* -------- ------------------------------------------------------
   Purpose: Return the result of the last operation
   Input:   -
   Output:  m_result QString
   -------- ------------------------------------------------------ */
QString MidiScriptEngine::getResult() {
    return m_result.toString();
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
   Purpose: Return the script engine
   Input:   -
   Output:  m_engine QScriptEngine
   -------- ------------------------------------------------------ */
QScriptEngine * MidiScriptEngine::getEngine() {
    return &m_engine;
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

        if (line.indexOf('#') != 0) {    // ignore # hashed out comments
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
    ControlObject *pot = ControlObject::getControl(ConfigKey(group, name));
    if (pot == NULL) {
        qDebug("MidiScriptEngine: Unknown control %s:%s", group, name);
        return 0.0;
    }
    return pot->get();
}

/* -------- ------------------------------------------------------
   Purpose: Sets new value of a Mixxx control (for scripts)
   Input:   Control group, Key name, new value
   Output:  -
   -------- ------------------------------------------------------ */
void MidiScriptEngine::setValue(QString group, QString name, double newValue) {
    ControlObject *pot = ControlObject::getControl(ConfigKey(group, name));
    pot->queueFromThread(newValue);
}
