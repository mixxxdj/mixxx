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
    ~MidiScriptEngine();

    QScriptValue engineGlobalObject;
    QScriptEngine *getEngine();

    void clearCode();
    bool loadScript(QString filepath);
    QString getResult();
    QString getLastFilepath();

    void evaluateScript();
    bool isGood();
    QScriptValue execute(QString);
    QStringList getFunctionList();

    bool checkException();

    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE bool connectControl(QString group, QString name, QString function, bool disconnect = false);

private:
    QScriptEngine m_engine;
    QScriptValue m_result;
    QString m_lastFilepath;
    QString m_scriptCode;
    bool m_scriptGood;
};
#endif

