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

    void setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) override {
        auto pMidiMapping = std::dynamic_pointer_cast<LegacyMidiControllerMapping>(pMapping);
        if (pMidiMapping) {
            m_bMidiMapping = true;
            m_bHidMapping = false;
            m_pMidiMapping = pMidiMapping;
            m_pHidMapping = nullptr;
            return;
        }

        auto pHidMapping = std::dynamic_pointer_cast<LegacyHidControllerMapping>(pMapping);
        if (pHidMapping) {
            m_bMidiMapping = false;
            m_bHidMapping = true;
            m_pMidiMapping = nullptr;
            m_pHidMapping = pHidMapping;
        }
    }

    virtual std::shared_ptr<LegacyControllerMapping> cloneMapping() override {
        if (m_pMidiMapping) {
            return m_pMidiMapping->clone();
        } else if (m_pHidMapping) {
            return m_pHidMapping->clone();
        }
        return nullptr;
    };

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
    bool m_bMidiMapping;
    bool m_bHidMapping;
    std::shared_ptr<LegacyMidiControllerMapping> m_pMidiMapping;
    std::shared_ptr<LegacyHidControllerMapping> m_pHidMapping;
};

class LegacyControllerMappingValidationTest : public MixxxTest {
  protected:
    void SetUp() override;

    bool testLoadMapping(const MappingInfo& mapping);

    QDir m_mappingPath;
    QScopedPointer<MappingInfoEnumerator> m_pEnumerator;
};
