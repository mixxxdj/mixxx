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

/* -------- ------------------------------------------------------
   Purpose: Open default script file, read into QString, evaluate() it
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
// MidiScriptEngine::MidiScriptEngine() {
// 
// 	// Default common script file
// 	MidiScriptEngine(UNIX_SHARE_PATH "/midi/midi-mappings-scripts.js");
// }

/* -------- ------------------------------------------------------
   Purpose: Open script file, read into QString, evaluate() it
   Input:   Path to script file
   Output:  -
   -------- ------------------------------------------------------ */
MidiScriptEngine::MidiScriptEngine(QString filepath) : m_engine() {

	m_filepath=filepath;
// 	qDebug() << "MidiScriptEngine: Path construction";

	QScriptValue globalObject = m_engine.globalObject();
	globalObject.setProperty("Mixxx", m_engine.newQObject(this));
	
	// Read in the script file
	QFile input(m_filepath);
	if (!input.open(QIODevice::ReadOnly)) {
		qWarning() << "MidiScriptEngine: Problem opening the script file: " << m_filepath << ", error #" << input.error();
		return;
	}
	m_scriptCode = QString(input.readAll());
	input.close();
}

MidiScriptEngine::~MidiScriptEngine() {
}

/* -------- ------------------------------------------------------
   Purpose: Validate script syntax, then evaluate() it so the
            functions are registered & available for use.
   Input:   -
   Output:  m_result QString is set
   -------- ------------------------------------------------------ */
bool MidiScriptEngine::evaluateScript() {
	if (!m_engine.canEvaluate(m_scriptCode)) {
		qWarning() << "MidiScriptEngine: ?Syntax error in script file:" << m_filepath;
		m_scriptCode.clear();	// Free up now-unneeded memory
		return false;
	}
	m_result = m_engine.evaluate(m_scriptCode).toString();
	qDebug() << "MidiScriptEngine: Script" << m_filepath << "evaluated successfully.";
// 	qDebug() << "m_scriptCode: " << m_scriptCode;
	return true;
}

/* -------- ------------------------------------------------------
   Purpose: Return the result of the last operation
   Input:   -
   Output:  m_result QString
   -------- ------------------------------------------------------ */
QString MidiScriptEngine::getResult() {
	return m_result;
}

/* -------- ------------------------------------------------------
   Purpose: Return the file path of the current script
   Input:   -
   Output:  m_result QString
   -------- ------------------------------------------------------ */
QString MidiScriptEngine::getFilepath() {
	return m_filepath;
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
	
// 	qDebug() << "MidiScriptEngine: m_scriptCode=" << m_scriptCode;	
	
	qDebug() << "MidiScriptEngine:" << codeLines.count() << "lines of code being searched for functions";
	
	// grep 'function' midi/midi-mappings-scripts.js|grep -i '(msg)'|sed -e 's/function \(.*\)(msg).*/\1/i' -e 's/[= ]//g'
	QRegExp rx("*function*(msg)*");	// Find all lines with function names in them
	rx.setPatternSyntax(QRegExp::Wildcard);
	
	int position = codeLines.indexOf(rx);

	while (position != -1) {	// While there are more matches
	
		QString line = codeLines.takeAt(position);	// Pull & remove the current match from the list.
		
		if (line.indexOf('#') != 0) {	// ignore # hashed out comments
			QStringList field = line.split(" ");
			qDebug() << "MidiScriptEngine: Found function:" << field[0] << "at line" << position;
			functionList.append(field[0]);
		}
		position = codeLines.indexOf(rx);
	}

	return functionList;
}
