#include <QObject>

#include "controllers/controller.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/hid/legacyhidcontrollermapping.h"
#include "controllers/midi/legacymidicontrollermapping.h"
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

    QString mappingExtension() override {
        // Doesn't affect anything at the moment.
        return ".test.xml";
    }

    ControllerJSProxy* jsProxy() override {
        return new FakeControllerJSProxy();
    }

    LegacyControllerMappingPointer getMapping() const override;

    void visit(const LegacyMidiControllerMapping* mapping) override {
        m_bMidiMapping = true;
        m_bHidMapping = false;
        m_midiMapping = *mapping;
        m_hidMapping = LegacyHidControllerMapping();
    }

    void visit(const LegacyHidControllerMapping* mapping) override {
        m_bMidiMapping = false;
        m_bHidMapping = true;
        m_midiMapping = LegacyMidiControllerMapping();
        m_hidMapping = *mapping;
    }

    void accept(ControllerVisitor* visitor) override {
        // Do nothing since we aren't a normal controller.
        Q_UNUSED(visitor);
    }

    bool isMappable() const override;

    bool matchMapping(const MappingInfo& mapping) override {
        // We're not testing product info matching in this test.
        Q_UNUSED(mapping);
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
    LegacyControllerMapping* mapping() override;

    bool m_bMidiMapping;
    bool m_bHidMapping;
    LegacyMidiControllerMapping m_midiMapping;
    LegacyHidControllerMapping m_hidMapping;
};

class LegacyControllerMappingValidationTest : public MixxxTest {
  protected:
    void SetUp() override;

    bool testLoadMapping(const MappingInfo& mapping);

    QDir m_mappingPath;
    QScopedPointer<MappingInfoEnumerator> m_pEnumerator;
};
