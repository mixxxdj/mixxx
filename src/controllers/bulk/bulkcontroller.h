/**
  * @file bulkcontroller.h
  * @author Neale Picket  neale@woozle.org
  * @date Thu Jun 28 2012
  * @brief USB Bulk controller backend
  */

#ifndef BULKCONTROLLER_H
#define BULKCONTROLLER_H

#include <QAtomicInt>

#include "controllers/controller.h"
#include "controllers/hid/hidcontrollerpreset.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"
#include "util/duration.h"

struct libusb_device_handle;
struct libusb_context;
struct libusb_device_descriptor;

class BulkReader : public QThread {
    Q_OBJECT
  public:
    BulkReader(libusb_device_handle *handle, unsigned char in_epaddr);
    virtual ~BulkReader();

    void stop();

  signals:
    void incomingData(QByteArray data, mixxx::Duration timestamp);

  protected:
    void run();

  private:
    libusb_device_handle* m_phandle;
    QAtomicInt m_stop;
    unsigned char m_in_epaddr;
};

class BulkController : public Controller {
    Q_OBJECT
  public:
    BulkController(UserSettingsPointer pConfig,
            libusb_context* context,
            libusb_device_handle* handle,
            struct libusb_device_descriptor* desc);
    ~BulkController() override;

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

  protected:
    Q_INVOKABLE void send(QList<int> data, unsigned int length);

  private slots:
    int open() override;
    int close() override;

  private:
    // For devices which only support a single report, reportID must be set to
    // 0x0.
    void send(QByteArray data) override;

    // Returns a pointer to the currently loaded controller preset. For internal
    // use only.
    ControllerPreset* preset() override {
        return &m_preset;
    }

    bool matchProductInfo(const ProductInfo& product);

    libusb_context* m_context;
    libusb_device_handle *m_phandle;

    // Local copies of things we need from desc

    unsigned short vendor_id;
    unsigned short product_id;
    unsigned char in_epaddr;
    unsigned char out_epaddr;
    QString manufacturer;
    QString product;

    QString m_sUID;
    BulkReader* m_pReader;
    HidControllerPreset m_preset;
};

#endif
