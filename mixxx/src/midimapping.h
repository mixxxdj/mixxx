/***************************************************************************
                             midimapping.h
                           MIDI Mapping Class
                           -------------------
    begin                : Sat Jan 17 2009
    copyright            : (C) 2009 Sean M. Pappalardo
    email                : pegasus@c64.org

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MIDIMAPPING_H
#define MIDIMAPPING_H

#include "midiobject.h"

#ifdef __SCRIPT__
#include "script/midiscriptengine.h"
#endif

class MidiObject;     // Forward declaration


#define BINDINGS_PATH QDir::homePath().append("/").append(".MixxxMIDIBindings.xml")

class MidiMapping : public QObject
{
    public:
        /** Constructor also loads & applies the default XML MIDI mapping file */
        MidiMapping(MidiObject* midi_object);
        ~MidiMapping();
        
        void loadPreset(QString path);
        void loadPreset(QDomElement root);
        
        QList<QHash<QString,QString> >* getRowParams();
        QList<QHash<QString,QString> >* getOutputRowParams();
        void deleteRowParams();
        
        void savePreset(QString path = BINDINGS_PATH);
        void applyPreset();
        void clearPreset();
        
        void addControl(QDomElement control, QString device);
        void addOutput(QDomElement output, QString device);

    private:
#ifdef __SCRIPT__
        /** Adds a script file name and function prefix to the list to be loaded */
        void addScriptFile(QString filename, QString functionprefix);

        QList<QString> m_pScriptFileNames;
        QList<QString> m_pScriptFunctionPrefixes;
        MidiScriptEngine *m_pScriptEngine;
#endif
        QDomElement m_pBindings;
        MidiObject* m_pMidiObject;
        QList<QHash<QString,QString> > m_pAddRowParams;
        QList<QHash<QString,QString> > m_pAddOutputRowParams;

ConfigObject<ConfigValueMidi> *m_pMidiConfig; // FIXME: added to make midimappings.cpp compile

};

#endif
