/**
* @file midicontroller.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Tue 7 Feb 2012
* @brief MIDI Controller base class
*
* This is a base class representing a MIDI controller.
*   It must be inherited by a class that implements it on some API.
*
*   Note that the subclass' destructor should call close() at a minimum.
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MIDICONTROLLER_H
#define MIDICONTROLLER_H

#include "controllers/controller.h"
#include "midioutputhandler.h"
#include "softtakeover.h"
#include "midicontrollerengine.h"

#include "midimessage.h"

class MidiController : public Controller {
Q_OBJECT    // For signals & slots
    
    friend class MidiOutputHandler; // So it can access sendShortMsg()

    public:
        MidiController();
        virtual ~MidiController();

    signals:
        void midiEvent(MidiKey message);

    protected:
        Q_INVOKABLE void sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2);
        Q_INVOKABLE void sendSysexMsg(QList<int> data, unsigned int length) { Controller::send(data,length); }  // Alias

    private slots:
//         virtual int open() = 0;
        virtual int close() = 0;

        /** Initializes the engine and static output mappings. */
        void applyPreset();

    private:
        virtual void send(unsigned int word);
        void receive(char status, char control = 0, char value = 0);
        /** For System Exclusive message reception */
        void receive(const unsigned char data[], unsigned int length);

        void startEngine();

        double computeValue(MidiOptions options, double _prevmidivalue, double _newmidivalue);

        void loadPreset(QDomElement root, bool forceLoad=false);
        /** Updates the DOM with what is currently in the table */
        QDomDocument buildDomElement();
        void mappingToXML(QDomElement& parentNode, QString group, QString item, char status, uchar control) const;
        void outputMappingToXML(QDomElement& parentNode, char on, char off, float max, float min) const;

        void createOutputHandlers(QDomNode node);
        void updateAllOutputs();
        void destroyOutputHandlers();

        QDomElement m_bindings;
        QList<MidiOutputHandler*> m_outputs;

        QHash<uint16_t, QPair<ConfigKey, MidiOptions> > m_mappings;
        QHash<ConfigKey, MidiOutput> m_outputMappings;

        MidiControllerEngine *m_pEngine;

        SoftTakeover m_st;

        bool m_bMidiLearn;
};

#endif
