/**
  * @file bulkcontroller.h
  * @author Neale Picket  neale@woozle.org
  * @date Thu Jun 28 2012
  * @brief USB Bulk controller backend
  */

#ifndef BULKCONTROLLER_H
#define BULKCONTROLLER_H

#include <libusb.h>

#include <QAtomicInt>

#include "controllers/controller.h"
#include "controllers/hid/hidcontrollerpreset.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"

class BulkReader : public QThread {
    Q_OBJECT
  public:
    BulkReader(libusb_device_handle *handle, unsigned char in_epaddr);
    virtual ~BulkReader();

    void stop();

  signals:
    void incomingData(QByteArray data);

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
    BulkController(libusb_context* context, libusb_device_handle *handle,
                   struct libusb_device_descriptor *desc);
    virtual ~BulkController();

    virtual QString presetExtension();

    virtual ControllerPresetPointer getPreset() const {
        HidControllerPreset* pClone = new HidControllerPreset();
        *pClone = m_preset;
        return ControllerPresetPointer(pClone);
    }

    virtual bool savePreset(const QString fileName) const;

    virtual ControllerPresetFileHandler* getFileHandler() const {
        return new HidControllerPresetFileHandler();
    }

    virtual void visit(const MidiControllerPreset* preset);
    virtual void visit(const HidControllerPreset* preset);

    virtual bool isMappable() const {
        return m_preset.isMappable();
    }

    virtual bool matchPreset(const PresetInfo& preset);
    virtual bool matchProductInfo(QHash <QString,QString >);

  protected:
    Q_INVOKABLE void send(QList<int> data, unsigned int length);

  private slots:
    int open();
    int close();

  private:
    // For devices which only support a single report, reportID must be set to
    // 0x0.
    virtual void send(QByteArray data);

    virtual bool isPolling() const {
        return false;
    }

    // Returns a pointer to the currently loaded controller preset. For internal
    // use only.
    virtual ControllerPreset* preset() {
        return &m_preset;
    }

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
