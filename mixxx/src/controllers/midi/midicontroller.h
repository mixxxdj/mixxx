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
#include "controllers/midi/midioutputhandler.h"
#include "softtakeover.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midicontrollerpreset.h"
#include "controllers/midi/midicontrollerpresetfilehandler.h"

class MidiController : public Controller {
    Q_OBJECT
  public:
    MidiController();
    virtual ~MidiController();

    virtual QString presetExtension();
    inline QString defaultPreset();

    virtual const ControllerPreset* getPreset() const {
        // TODO(XXX) clone the preset
        return &m_preset;
    }

    virtual bool savePreset(const QString fileName) const;

    virtual ControllerPresetFileHandler* getFileHandler() const {
        return new MidiControllerPresetFileHandler();
    }

    virtual void visit(const MidiControllerPreset* preset);
    virtual void visit(const HidControllerPreset* preset);

  signals:
    void midiEvent(MidiKey message);

  protected:
    Q_INVOKABLE void sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2);
    // Alias for send()
    Q_INVOKABLE inline void sendSysexMsg(QList<int> data, unsigned int length) {
        Controller::send(data, length);
    }

  protected slots:
    void receive(unsigned char status, unsigned char control = 0,
                 unsigned char value = 0);
    // For receiving System Exclusive messages
    void receive(const QByteArray data);

  private slots:
    //virtual int open() = 0;
    virtual int close() = 0;

    // Initializes the engine and static output mappings.
    void applyPreset();

  private:
    virtual void send(unsigned int word);
    double computeValue(MidiOptions options, double _prevmidivalue, double _newmidivalue);
    void createOutputHandlers();
    void updateAllOutputs();
    void destroyOutputHandlers();

    QList<MidiOutputHandler*> m_outputs;
    MidiControllerPreset m_preset;
    SoftTakeover m_st;
    bool m_bMidiLearn;

    // So it can access sendShortMsg()
    friend class MidiOutputHandler;
};

#endif
