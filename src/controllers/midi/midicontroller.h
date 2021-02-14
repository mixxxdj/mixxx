#pragma once

#include "controllers/controller.h"
#include "controllers/midi/legacymidicontrollermapping.h"
#include "controllers/midi/legacymidicontrollermappingfilehandler.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midioutputhandler.h"
#include "controllers/softtakeover.h"

class DlgControllerLearning;

/// MIDI Controller base class
///
/// This is a base class representing a MIDI controller.
/// It must be inherited by a class that implements it on some API.
///
/// Note that the subclass' destructor should call close() at a minimum.
class MidiController : public Controller {
    Q_OBJECT
  public:
    explicit MidiController();
    ~MidiController() override;

    ControllerJSProxy* jsProxy() override;

    QString mappingExtension() override;

    LegacyControllerMappingPointer getMapping() const override {
        LegacyMidiControllerMapping* pClone = new LegacyMidiControllerMapping();
        *pClone = m_mapping;
        return LegacyControllerMappingPointer(pClone);
    }

    void visit(const LegacyMidiControllerMapping* mapping) override;
    void visit(const LegacyHidControllerMapping* mapping) override;

    void accept(ControllerVisitor* visitor) override {
        if (visitor) {
            visitor->visit(this);
        }
    }

    bool isMappable() const override {
        return m_mapping.isMappable();
    }

    bool matchMapping(const MappingInfo& mapping) override;

  signals:
    void messageReceived(unsigned char status, unsigned char control, unsigned char value);

  protected:
    virtual void sendShortMsg(unsigned char status,
            unsigned char byte1,
            unsigned char byte2) = 0;

    /// Alias for send()
    /// The length parameter is here for backwards compatibility for when scripts
    /// were required to specify it.
    inline void sendSysexMsg(const QList<int>& data, unsigned int length = 0) {
        Q_UNUSED(length);
        send(data);
    }

  protected slots:
    virtual void receivedShortMessage(
            unsigned char status,
            unsigned char control,
            unsigned char value,
            mixxx::Duration timestamp);
    // For receiving System Exclusive messages
    void receive(const QByteArray& data, mixxx::Duration timestamp) override;
    int close() override;

  private slots:
    bool applyMapping() override;

    void learnTemporaryInputMappings(const MidiInputMappings& mappings);
    void clearTemporaryInputMappings();
    void commitTemporaryInputMappings();

  private:
    void processInputMapping(
            const MidiInputMapping& mapping,
            unsigned char status,
            unsigned char control,
            unsigned char value,
            mixxx::Duration timestamp);
    void processInputMapping(
            const MidiInputMapping& mapping,
            const QByteArray& data,
            mixxx::Duration timestamp);

    double computeValue(MidiOptions options, double _prevmidivalue, double _newmidivalue);
    void createOutputHandlers();
    void updateAllOutputs();
    void destroyOutputHandlers();

    /// Returns a pointer to the currently loaded controller mapping. For internal
    /// use only.
    LegacyControllerMapping* mapping() override {
        return &m_mapping;
    }

    QHash<uint16_t, MidiInputMapping> m_temporaryInputMappings;
    QList<MidiOutputHandler*> m_outputs;
    LegacyMidiControllerMapping m_mapping;
    SoftTakeoverCtrl m_st;
    QList<QPair<MidiInputMapping, unsigned char> > m_fourteen_bit_queued_mappings;

    // So it can access sendShortMsg()
    friend class MidiOutputHandler;
    friend class MidiControllerTest;
    friend class MidiControllerJSProxy;

    // MIDI learning assistant
    friend class DlgControllerLearning;
};

class MidiControllerJSProxy : public ControllerJSProxy {
    Q_OBJECT
  public:
    MidiControllerJSProxy(MidiController* m_pController)
            : ControllerJSProxy(m_pController),
              m_pMidiController(m_pController) {
    }

    Q_INVOKABLE void sendShortMsg(unsigned char status,
            unsigned char byte1,
            unsigned char byte2) {
        m_pMidiController->sendShortMsg(status, byte1, byte2);
    }

    Q_INVOKABLE void sendSysexMsg(const QList<int>& data, unsigned int length = 0) {
        m_pMidiController->sendSysexMsg(data, length);
    }

  private:
    MidiController* m_pMidiController;
};
