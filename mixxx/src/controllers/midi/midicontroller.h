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

#ifndef MIDICONTROLLER_H
#define MIDICONTROLLER_H

#include "controllers/controller.h"
#include "controllers/midi/midicontrollerpreset.h"
#include "controllers/midi/midicontrollerpresetfilehandler.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midioutputhandler.h"
#include "controllers/softtakeover.h"

class MidiController : public Controller {
    Q_OBJECT
  public:
    MidiController();
    virtual ~MidiController();

    virtual QString presetExtension();
    inline QString defaultPreset();

    virtual ControllerPresetPointer getPreset() const {
        MidiControllerPreset* pClone = new MidiControllerPreset();
        *pClone = m_preset;
        return ControllerPresetPointer(pClone);
    }

    virtual bool savePreset(const QString fileName) const;

    virtual ControllerPresetFileHandler* getFileHandler() const {
        return new MidiControllerPresetFileHandler();
    }

    virtual void visit(const MidiControllerPreset* preset);
    virtual void visit(const HidControllerPreset* preset);

    virtual bool isMappable() const {
        return m_preset.isMappable();
    }

    virtual bool matchPreset(const PresetInfo& preset);

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
    virtual int close();

    void clearInputMappings();
    void clearOutputMappings();

  private slots:
    // Initializes the engine and static output mappings.
    void applyPreset(QString configPath);

  private:
    virtual void send(unsigned int word) = 0;
    double computeValue(MidiOptions options, double _prevmidivalue, double _newmidivalue);
    void createOutputHandlers();
    void updateAllOutputs();
    void destroyOutputHandlers();

    // Returns a pointer to the currently loaded controller preset. For internal
    // use only.
    virtual ControllerPreset* preset() {
        return &m_preset;
    }

    QList<MidiOutputHandler*> m_outputs;
    MidiControllerPreset m_preset;
    SoftTakeover m_st;

    // So it can access sendShortMsg()
    friend class MidiOutputHandler;
};

#endif
