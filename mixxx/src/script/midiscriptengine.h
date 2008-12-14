/***************************************************************************
                          midiscriptengine.h  -  description
                          -------------------
    begin                : Fri Dec 12 2008
    copyright            : (C) 2008 by Sean M. Pappalardo
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
#ifndef MIDISCRIPTENGINE_H
#define MIDISCRIPTENGINE_H

#include <QtScript>

class MidiScriptEngine : public QObject {

	Q_OBJECT

public:
	MidiScriptEngine();
	MidiScriptEngine(QString);
	~MidiScriptEngine();

	QString getResult();
	QString getFilepath();

	bool evaluateScript();
	QStringList getFunctionList();
	
private:
	QScriptEngine m_engine;
	QString m_filepath;
	QString m_scriptCode;
	QString m_result;
};

#endif

