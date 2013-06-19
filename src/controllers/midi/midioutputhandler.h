/**
 * @file midioutputhandler.h
 * @author Sean Pappalardo spappalardo@mixxx.org
 * @date Tue 11 Feb 2012
 * @brief Static MIDI output mapping handler
 *
 * This class listens to a control object and sends a midi message based on the
 * value.
 */

#ifndef MIDIOUTPUTHANDLER_H
#define MIDIOUTPUTHANDLER_H

#include "controlobjectthread.h"

class MidiController;   // forward declaration

class MidiOutputHandler : QObject {
    Q_OBJECT
  public:
    MidiOutputHandler(QString group, QString key, MidiController* controller,
                      float min, float max,
                      unsigned char status, unsigned char midino,
                      unsigned char on, unsigned char off);
    virtual ~MidiOutputHandler();

    bool validate();
    void update();

  public slots:
    void controlChanged(double value);

  private:
    MidiController* m_pController;
    ControlObjectThread m_cobj;
    float m_min;
    float m_max;
    unsigned char m_status;
    unsigned char m_midino;
    unsigned char m_on;
    unsigned char m_off;
    double m_lastVal;
};

#endif

