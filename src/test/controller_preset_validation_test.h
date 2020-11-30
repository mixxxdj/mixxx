#include <QObject>

#include "controllers/controller.h"
#include "controllers/controllerpresetinfoenumerator.h"
#include "controllers/hid/hidcontrollerpreset.h"
#include "controllers/midi/midicontrollerpreset.h"
#include "test/mixxxtest.h"

class FakeControllerJSProxy : public ControllerJSProxy {
    Q_OBJECT
  public:
    FakeControllerJSProxy();

    Q_INVOKABLE void send(const QList<int>& data, unsigned int length = 0) override;

    Q_INVOKABLE void sendSysexMsg(const QList<int>& data, unsigned int length = 0);

    Q_INVOKABLE void sendShortMsg(unsigned char status,
            unsigned char byte1,
            unsigned char byte2);
};

class FakeController : public Controller {
    Q_OBJECT
  public:
    FakeController();
    ~FakeController() override;

    QString presetExtension() override {
        // Doesn't affect anything at the moment.
        return ".test.xml";
    }

    ControllerJSProxy* jsProxy() override {
        return new FakeControllerJSProxy();
    }

    ControllerPresetPointer getPreset() const override;

    void visit(const MidiControllerPreset* preset) override {
        m_bMidiPreset = true;
        m_bHidPreset = false;
        m_midiPreset = *preset;
        m_hidPreset = HidControllerPreset();
    }

    void visit(const HidControllerPreset* preset) override {
        m_bMidiPreset = false;
        m_bHidPreset = true;
        m_midiPreset = MidiControllerPreset();
        m_hidPreset = *preset;
    }

    void accept(ControllerVisitor* visitor) override {
        // Do nothing since we aren't a normal controller.
        Q_UNUSED(visitor);
    }

    bool isMappable() const override;

    bool matchPreset(const PresetInfo& preset) override {
        // We're not testing product info matching in this test.
        Q_UNUSED(preset);
        return false;
    }

  protected:
    void send(const QList<int>& data, unsigned int length) override {
        Q_UNUSED(data);
        Q_UNUSED(length);
    }

    void sendBytes(const QByteArray& data) override {
        Q_UNUSED(data);
    }

  private slots:
    int open() override {
        return 0;
    }

    int close() override {
        return 0;
    }

  private:
    ControllerPreset* preset() override;

    bool m_bMidiPreset;
    bool m_bHidPreset;
    MidiControllerPreset m_midiPreset;
    HidControllerPreset m_hidPreset;
};

class ControllerPresetValidationTest : public MixxxTest {
  protected:
    void SetUp() override;

    bool testLoadPreset(const PresetInfo& preset);

    QDir m_presetPath;
    QScopedPointer<PresetInfoEnumerator> m_pEnumerator;
};
