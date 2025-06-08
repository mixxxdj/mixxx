#pragma once

#include <QObject>

#include "controllers/controller.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/hid/legacyhidcontrollermapping.h"
#include "controllers/midi/legacymidicontrollermapping.h"
#include "test/mixxxtest.h"

class FakeMidiControllerJSProxy : public ControllerJSProxy {
    Q_OBJECT
  public:
    FakeMidiControllerJSProxy();

    Q_INVOKABLE void send(const QList<int>& data, unsigned int length = 0) override;

    Q_INVOKABLE void sendSysexMsg(const QList<int>& data, unsigned int length = 0);

    Q_INVOKABLE void sendShortMsg(unsigned char status,
            unsigned char byte1,
            unsigned char byte2);
};

class FakeHidControllerJSProxy : public ControllerJSProxy {
    Q_OBJECT
  public:
    FakeHidControllerJSProxy();

    Q_INVOKABLE void send(const QList<int>& data, unsigned int length = 0) override;
    Q_INVOKABLE void send(const QList<int>& dataList,
            unsigned int length,
            quint8 reportID,
            bool useNonSkippingFIFO = false);

    Q_INVOKABLE void sendOutputReport(quint8 reportID,
            const QByteArray& dataArray,
            bool resendUnchangedReport = false);
    Q_INVOKABLE QByteArray getInputReport(
            quint8 reportID);
    Q_INVOKABLE void sendFeatureReport(
            quint8 reportID, const QByteArray& reportData);
    Q_INVOKABLE QByteArray getFeatureReport(
            quint8 reportID);
};

class FakeBulkControllerJSProxy : public ControllerJSProxy {
    Q_OBJECT
  public:
    FakeBulkControllerJSProxy();

    Q_INVOKABLE void send(const QList<int>& data, unsigned int length = 0) override;
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
        if (this->m_bMidiMapping == true) {
            return new FakeMidiControllerJSProxy();
        }
        if (this->m_bHidMapping == true) {
            return new FakeHidControllerJSProxy();
        }
        // Bulk mapping
        return new FakeBulkControllerJSProxy();
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

    QList<LegacyControllerMapping::ScriptFileInfo> getMappingScriptFiles() override {
        if (m_pMidiMapping) {
            return m_pMidiMapping->getScriptFiles();
        } else if (m_pHidMapping) {
            return m_pHidMapping->getScriptFiles();
        }
        return {};
    }

    QList<std::shared_ptr<AbstractLegacyControllerSetting>> getMappingSettings() override {
        if (m_pMidiMapping) {
            return m_pMidiMapping->getSettings();
        } else if (m_pHidMapping) {
            return m_pHidMapping->getSettings();
        }
        return {};
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

  private:
    int open() override {
        return 0;
    }

    int close() override {
        return 0;
    }

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
