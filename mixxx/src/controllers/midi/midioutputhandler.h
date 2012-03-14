/**
* @file midioutputhandler.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Tue 11 Feb 2012
* @brief Static MIDI output mapping handler
*
* This class listens to a control object and sends a midi message based on the value
*
*/

#ifndef MIDIHOUTPUTHANDLER_H
#define MIDIHOUTPUTHANDLER_H

#include "controlobject.h"

class MidiController;   // forward declaration

class MidiOutputHandler : QObject {
    Q_OBJECT
    public:
        MidiOutputHandler(QString group, QString key, MidiController* controller,
                          float min, float max,
                          unsigned char status, unsigned char midino,
                          unsigned char on, unsigned char off);
        ~MidiOutputHandler();

        void update();

    public slots:
        void controlChanged(double value);

    private:
        ControlObject* m_cobj;
        float m_min;
        float m_max;
        unsigned char m_status;
        unsigned char m_midino;
        unsigned char m_on;
        unsigned char m_off;

        MidiController* m_pController;
        unsigned char m_controlno;
};

#endif

