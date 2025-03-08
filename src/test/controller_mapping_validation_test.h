#pragma once

#include <QObject>

#include "control/controlindicatortimer.h"
#include "controllers/controller.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/hid/legacyhidcontrollermapping.h"
#include "controllers/midi/legacymidicontrollermapping.h"
#include "library/trackcollectionmanager.h"
#include "test/mixxxdbtest.h"
#include "test/soundsourceproviderregistration.h"

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

    PhysicalTransportProtocol getPhysicalTransportProtocol() const override {
        return PhysicalTransportProtocol::USB;
    }
    DataRepresentationProtocol getDataRepresentationProtocol() const override {
        if (m_bMidiMapping) {
            return DataRepresentationProtocol::MIDI;
        } else if (m_bHidMapping) {
            return DataRepresentationProtocol::HID;
        } else {
            return DataRepresentationProtocol::USB_BULK_TRANSFER;
        }
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
            return std::make_shared<LegacyMidiControllerMapping>(*m_pMidiMapping);
        } else if (m_pHidMapping) {
            return std::make_shared<LegacyHidControllerMapping>(*m_pHidMapping);
        }
        return nullptr;
    };

    bool isMappable() const override;

    bool matchMapping(const MappingInfo& mapping) override {
        // We're not testing product info matching in this test.
        Q_UNUSED(mapping);
        return false;
    }
    QString getVendorString() const override {
        static const QString vendor = "Test Vendor";
        return vendor;
    }
    std::optional<uint16_t> getVendorId() const override {
        return std::nullopt;
    }

    QString getProductString() const override {
        static const QString product = "Test Product";
        return product;
    }
    std::optional<uint16_t> getProductId() const override {
        return std::nullopt;
    }

    QString getSerialNumber() const override {
        static const QString serialNumber = "123456789";
        return serialNumber;
    }

    std::optional<uint8_t> getUsbInterfaceNumber() const override {
        return std::nullopt;
    }

  protected:
    void send(const QList<int>& data, unsigned int length) override {
        Q_UNUSED(data);
        Q_UNUSED(length);
    }

    bool sendBytes(const QByteArray& data) override {
        Q_UNUSED(data);
        return true;
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

class EngineMixer;
class EffectsManager;
class SoundManager;
class RecordingManager;
class Library;
class PlayerManager;

// We can't inherit from LibraryTest because that creates a key_notation control object that is also
// created by the Library object itself. The duplicated CO creation causes a debug assert.
class LegacyControllerMappingValidationTest : public MixxxDbTest, SoundSourceProviderRegistration {
  public:
    LegacyControllerMappingValidationTest()
            : MixxxDbTest(true) {
    }

  protected:
    void SetUp() override;
#ifdef MIXXX_USE_QML
    void TearDown() override;

    TrackPointer getOrAddTrackByLocation(
            const QString& trackLocation) const {
        return m_pTrackCollectionManager->getOrAddTrack(
                TrackRef::fromFilePath(trackLocation));
    }

    std::shared_ptr<EffectsManager> m_pEffectsManager;
    std::shared_ptr<mixxx::ControlIndicatorTimer> m_pControlIndicatorTimer;
    std::shared_ptr<EngineMixer> m_pEngine;
    std::shared_ptr<SoundManager> m_pSoundManager;
    std::shared_ptr<PlayerManager> m_pPlayerManager;
    std::shared_ptr<TrackCollectionManager> m_pTrackCollectionManager;
    std::shared_ptr<RecordingManager> m_pRecordingManager;
    std::shared_ptr<Library> m_pLibrary;
#endif

    bool testLoadMapping(const MappingInfo& mapping);

    QDir m_mappingPath;
    QScopedPointer<MappingInfoEnumerator> m_pEnumerator;
};
