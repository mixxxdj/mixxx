/**
  * @file hidcontroller.h
  * @author Sean M. Pappalardo	spappalardo@mixxx.org
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

class HidReader : public QThread {
    Q_OBJECT
  public:
    HidReader(hid_device* device);
    virtual ~HidReader();

    void stop() {
        m_stop = 1;
    }

  signals:
    void incomingData(QByteArray data);

  protected:
    void run();

  private:
    hid_device* m_pHidDevice;
    QAtomicInt m_stop;
};

class HidController : public Controller {
    Q_OBJECT
  public:
    HidController(const hid_device_info deviceInfo);
    virtual ~HidController();

    virtual const ControllerPreset* getPreset() const {
        // TODO(XXX) clone the preset
        return &m_preset;
    }

    virtual bool savePreset(const QString fileName) const;

    virtual ControllerPresetFileHandler* getFileHandler() const {
        return new HidControllerPresetFileHandler();
    }

    virtual void visit(const MidiControllerPreset* preset);
    virtual void visit(const HidControllerPreset* preset);

  protected:
    Q_INVOKABLE void send(QList<int> data, unsigned int length, unsigned int reportID = 0);

  private slots:
    int open();
    int close();

  private:
    // For devices which only support a single report, reportID must be set to
    // 0x0.
    virtual void send(QByteArray data);
    virtual void send(QByteArray data, unsigned int reportID);

    virtual bool isPolling() const {
        return false;
    }

    // Local copies of things we need from hid_device_info
    int hid_interface_number;
    unsigned short hid_vendor_id;
    unsigned short hid_product_id;
    char* hid_path;
    wchar_t* hid_serial;
    QString hid_manufacturer;
    QString hid_product;

    QString m_sUID;
    hid_device* m_pHidDevice;
    HidReader* m_pReader;
    HidControllerPreset m_preset;
};

#endif
