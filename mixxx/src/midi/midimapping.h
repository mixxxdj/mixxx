/***************************************************************************
                             midimapping.h
                           MIDI Mapping Class
                           -------------------
    begin                : Sat Jan 17 2009
    copyright            : (C) 2009 Sean M. Pappalardo
                           (C) 2009 Albert Santoni
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

#include "mididevice.h"
#include "midimessage.h"
#include "mixxxcontrol.h"
#include "midiinputmapping.h"
#include "midioutputmapping.h"
#include <QTableWidget>

#ifdef __MIDISCRIPT__
#include "midiscriptengine.h"
#endif

//Forward declarations
class MidiInputMappingTableModel;
class MidiOutputMappingTableModel;

#define BINDINGS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("midi/")
// local midi mapping presets
#define LPRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("presets/")
#define MIDI_MAPPING_EXTENSION ".midi.xml"

class MidiMapping : public QObject
{
    Q_OBJECT

    public:
    /** Constructor also loads & applies the default XML MIDI mapping file */
    MidiMapping(MidiDevice* outputMidiDevice=NULL);
    ~MidiMapping();
    void setOutputMidiDevice(MidiDevice* outputMidiDevice);
    
    void setName(QString name);

    void loadPreset(bool forceLoad=false);
    void loadPreset(QString path, bool forceLoad=false);
    void loadPreset(QDomElement root, bool forceLoad=false);

    void savePreset();
    void savePreset(QString path);
    void applyPreset();
    
    
    MidiInputMappingTableModel* getMidiInputMappingTableModel();
    MidiOutputMappingTableModel* getMidiOutputMappingTableModel();
    //MixxxControl* getInputMixxxControl(MidiMessage command);

    static double ComputeValue(MidiOption midioption, double _prevmidivalue, double _newmidivalue);

    // MIDI Input Mapping Modifiers
    int numInputMidiMessages();
    bool isInputIndexValid(int index);
    bool isMidiMessageMapped(MidiMessage command);
    MidiMessage getInputMidiMessage(int index);
    MixxxControl getInputMixxxControl(int index);
    MixxxControl getInputMixxxControl(MidiMessage command);
    void setInputMidiMapping(MidiMessage command, MixxxControl control);
    void clearInputMidiMapping(int index);
    void clearInputMidiMapping(MidiMessage command);
    void clearInputMidiMapping(int index, int count);

    // MIDI Output Mapping Modifiers
    int numOutputMixxxControls();
    bool isOutputIndexValid(int index);
    bool isMixxxControlMapped(MixxxControl control);
    MixxxControl getOutputMixxxControl(int index);
    MidiMessage getOutputMidiMessage(int index);
    MidiMessage getOutputMidiMessage(MixxxControl control);
    void setOutputMidiMapping(MixxxControl control, MidiMessage command);
    void clearOutputMidiMapping(int index);
    void clearOutputMidiMapping(MixxxControl control);
    void clearOutputMidiMapping(int index, int count);
#ifdef __MIDISCRIPT__
    void initializeScripts();
    void startupScriptEngine();
    void shutdownScriptEngine();
    void restartScriptEngine();
    MidiScriptEngine *getMidiScriptEngine() { return m_pScriptEngine; };
#endif

public slots:
    void finishMidiLearn(MidiMessage message);
    void beginMidiLearn(MixxxControl control);
    void cancelMidiLearn();
    void slotScriptEngineReady();
    /** Restarts the script engine and re-applies the mapping
        to effectively reset the controller */
    void reset();
    
signals:
    void inputMappingChanged();
    void inputMappingChanged(int startIndex, int endIndex);
    void outputMappingChanged();
    void outputMappingChanged(int startIndex, int endIndex);
    void midiLearningStarted();
    void midiLearningFinished(MidiMessage);
    void midiLearningFinished();
    void callMidiScriptFunction(QString);
    void callMidiScriptFunction(QString, QString);
    void loadMidiScriptFiles(QList<QString>);
    void initMidiScripts(QList<QString>);
    void shutdownMidiScriptEngine(QList<QString>);

private:
    int internalNumInputMidiMessages();
    bool internalIsInputIndexValid(int index);
    void internalSetInputMidiMapping(MidiMessage command, 
                                     MixxxControl control,
                                     bool shouldEmit);
    int internalNumOutputMidiMessages();
    int internalNumOutputMixxxControls();
    bool internalIsOutputIndexValid(int index);
    void internalSetOutputMidiMapping(MixxxControl control,
                                      MidiMessage command,
                                      bool shouldEmit);
    void clearPreset();
    QDomDocument buildDomElement();
    void addControl(QDomElement& control, QString device);
    void addOutput(QDomElement& output, QString device);
    void addMidiScriptInfo(QDomElement &scriptFile, QString device); //Sucks

    bool addInputControl(MidiStatusByte midiStatus, int midiNo, int midiChannel,
                         QString controlObjectGroup, QString controlObjectKey,
                         QString controlObjectDescription, MidiOption midiOption);
    bool addInputControl(MidiMessage message, MixxxControl control);
    void removeInputMapping(MidiStatusByte midiStatus, int midiNo, int midiChannel);

#ifdef __MIDISCRIPT__
    /** Adds a script file name and function prefix to the list to be loaded */
    void addScriptFile(QString filename, QString functionprefix);
    /** Actually loads script code from the files in the list */
    void loadScriptCode();

    QList<QString> m_scriptFileNames;
    QList<QString> m_scriptFunctionPrefixes;
    MidiScriptEngine *m_pScriptEngine;

    QMutex m_scriptEngineInitializedMutex;
    QWaitCondition m_scriptEngineInitializedCondition;
#endif
    QMutex m_mappingLock;
    QDomElement m_Bindings;
    MidiInputMapping m_inputMapping;
    MidiOutputMapping m_outputMapping;
    MidiInputMappingTableModel* m_pMidiInputMappingTableModel;
    MidiOutputMappingTableModel* m_pMidiOutputMappingTableModel;
    MixxxControl m_controlToLearn;
    QString m_deviceName; /** Name of the device to look for in the <controller> XML nodes... */
    MidiDevice* m_pOutputMidiDevice; /** We need a pointer back to an _output_ MIDI device so our
                                         LED handlers know where to fire MIDI messages. Note that
                                         this can be NULL if there is no output MIDI device! */
};

#endif
