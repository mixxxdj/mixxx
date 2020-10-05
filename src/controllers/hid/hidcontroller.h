/**
  * @file hidcontroller.h
  * @author Sean M. Pappalardo  spappalardo@mixxx.org
  * @date Sun May 1 2011
  * @brief HID controller backend
  */

#ifndef HIDCONTROLLER_H
#define HIDCONTROLLER_H

#include <hidapi.h>

#include <QAtomicInt>

#include "controllers/controller.h"
#include "controllers/hid/hidcontrollerpreset.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"
#include "util/duration.h"

class HidController final : public Controller {
    Q_OBJECT
  public:
    HidController(const hid_device_info& deviceInfo);
    ~HidController() override;

    ControllerJSProxy* jsProxy() override;

    QString presetExtension() override;

    ControllerPresetPointer getPreset() const override {
        HidControllerPreset* pClone = new HidControllerPreset();
        *pClone = m_preset;
        return ControllerPresetPointer(pClone);
    }

    void visit(const MidiControllerPreset* preset) override;
    void visit(const HidControllerPreset* preset) override;

    void accept(ControllerVisitor* visitor) override {
        if (visitor) {
            visitor->visit(this);
        }
    }

    bool isMappable() const override {
        return m_preset.isMappable();
    }

    bool matchPreset(const PresetInfo& preset) override;

    static QString safeDecodeWideString(const wchar_t* pStr, size_t max_length);

  protected:
    void sendReport(QList<int> data, unsigned int length, unsigned int reportID);

  private slots:
    int open() override;
    int close() override;

    bool poll() override;
    bool isPolling() const override;

  private:
    // For devices which only support a single report, reportID must be set to
    // 0x0.
    void sendBytes(const QByteArray& data) override;
    void sendBytesReport(QByteArray data, unsigned int reportID);

    // Returns a pointer to the currently loaded controller preset. For internal
    // use only.
    ControllerPreset* preset() override {
        return &m_preset;
    }

    bool matchProductInfo(const ProductInfo& product);
    void guessDeviceCategory();

    // Local copies of things we need from hid_device_info
    int hid_interface_number;
    unsigned short hid_vendor_id;
    unsigned short hid_product_id;
    unsigned short hid_usage_page;
    unsigned short hid_usage;
    char* hid_path;
    wchar_t* hid_serial_raw;
    QString hid_serial;
    QString hid_manufacturer;
    QString hid_product;

    QString m_sUID;
    hid_device* m_pHidDevice;
    HidControllerPreset m_preset;

    static constexpr int kNumBuffers = 2;
    static constexpr int kBufferSize = 255;
    unsigned char m_pPollData[kNumBuffers][kBufferSize];
    int m_iPollingBufferIndex;

    friend class HidControllerJSProxy;
};

class HidControllerJSProxy : public ControllerJSProxy {
    Q_OBJECT
  public:
    HidControllerJSProxy(HidController* m_pController)
            : ControllerJSProxy(m_pController),
              m_pHidController(m_pController) {
    }

    Q_INVOKABLE void send(QList<int> data, unsigned int length = 0) override {
        m_pHidController->send(data, length);
    }

    Q_INVOKABLE void send(QList<int> data, unsigned int length, unsigned int reportID) {
        m_pHidController->sendReport(data, length, reportID);
    }

  private:
    HidController* m_pHidController;
};

#endif
