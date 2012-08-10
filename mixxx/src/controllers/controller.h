/**
* @file controller.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief Base class representing a physical (or software) controller.
*
* This is a base class representing a physical (or software) controller.  It
* must be inherited by a class that implements it on some API. Note that the
* subclass' destructor should call close() at a minimum.
*/

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "controllers/controllerengine.h"
#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetinfo.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/controllerpresetfilehandler.h"
#include "controllers/mixxxcontrol.h"

class Controller : public QObject, ControllerPresetVisitor {
    Q_OBJECT
  public:
    Controller();
    virtual ~Controller();  // Subclass should call close() at minimum.

    // Returns the extension for the controller (type) preset files.  This is
    // used by the ControllerManager to display only relevant preset files for
    // the controller (type.)
    virtual QString presetExtension() = 0;
    inline QString defaultPreset();

    void setPreset(const ControllerPreset& preset) {
        // We don't know the specific type of the preset so we need to ask
        // the preset to call our visitor methods with its type.
        preset.accept(this);
    }

    virtual bool savePreset(const QString filename) const = 0;

    // Returns a clone of the Controller's loaded preset.
    virtual ControllerPresetPointer getPreset() const = 0;
    virtual ControllerPresetFileHandler* getFileHandler() const = 0;

    inline bool isOpen() const {
        return m_bIsOpen;
    }
    inline bool isOutputDevice() const {
        return m_bIsOutputDevice;
    }
    inline bool isInputDevice() const {
        return m_bIsInputDevice;
    }
    inline QString getName() const {
        return m_sDeviceName;
    }
    inline QString getCategory() const {
        return m_sDeviceCategory;
    }
    inline bool debugging() const {
        return m_bDebug;
    }
    virtual bool isMappable() const = 0;
    inline bool isLearning() const {
        return m_bLearning;
    }

    virtual bool matchPreset(const PresetInfo& preset) = 0;

  signals:
    void learnedMessage(QString message);
    // Emitted when a new preset is loaded. pPreset is a /clone/ of the loaded
    // preset, not a pointer to the preset itself.
    void presetLoaded(ControllerPresetPointer pPreset);

  // Making these slots protected/private ensures that other parts of Mixxx can
  // only signal them which allows us to use no locks.
  protected slots:
    // Handles packets of raw bytes and passes them to an ".incomingData" script
    // function that is assumed to exist. (Sub-classes may want to reimplement
    // this if they have an alternate way of handling such data.)
    virtual void receive(const QByteArray data);

    // Initializes the controller engine
    virtual void applyPreset(QString resourcePath);

    void learn(MixxxControl control);
    void cancelLearn();

    virtual void clearInputMappings() {}
    virtual void clearOutputMappings() {}

  protected:
    Q_INVOKABLE void send(QList<int> data, unsigned int length);

    // To be called in sub-class' open() functions after opening the device but
    // before starting any input polling/processing.
    void startEngine();

    // To be called in sub-class' close() functions after stopping any input
    // polling/processing but before closing the device.
    void stopEngine();

    inline ControllerEngine* getEngine() const {
        return m_pEngine;
    }
    inline void setDeviceName(QString deviceName) {
        m_sDeviceName = deviceName;
    }
    inline void setDeviceCategory(QString deviceCategory) {
        m_sDeviceCategory = deviceCategory;
    }
    inline void setOutputDevice(bool outputDevice) {
        m_bIsOutputDevice = outputDevice;
    }
    inline void setInputDevice(bool inputDevice) {
        m_bIsInputDevice = inputDevice;
    }
    inline void setOpen(bool open) {
        m_bIsOpen = open;
    }
    inline MixxxControl controlToLearn() const {
        return m_controlToLearn;
    }
    inline void setControlToLearn(MixxxControl control) {
        m_controlToLearn = control;
    }


  private slots:
    virtual int open() = 0;
    virtual int close() = 0;
    // Requests that the device poll if it is a polling device. Returns true
    // if events were handled.
    virtual bool poll() { return false; }

  private:
    // This must be reimplemented by sub-classes desiring to send raw bytes to a
    // controller.
    virtual void send(QByteArray data) = 0;

    // Returns true if this device should receive polling signals via calls to
    // its poll() method.
    virtual bool isPolling() const = 0;

    // Returns a pointer to the currently loaded controller preset. For internal
    // use only.
    virtual ControllerPreset* preset() = 0;
    ControllerEngine* m_pEngine;

    // Verbose and unique device name suitable for display.
    QString m_sDeviceName;
    // Verbose and unique description of device type, defaults to empty
    QString m_sDeviceCategory;
    // Flag indicating if this device supports output (receiving data from
    // Mixxx)
    bool m_bIsOutputDevice;
    // Flag indicating if this device supports input (sending data to Mixxx)
    bool m_bIsInputDevice;
    // Indicates whether or not the device has been opened for input/output.
    bool m_bIsOpen;
    // Specifies whether or not we should dump incoming data to the console at
    // runtime. This is useful for end-user debugging and script-writing.
    bool m_bDebug;
    bool m_bLearning;
    MixxxControl m_controlToLearn;

    friend class ControllerManager; // accesses lots of our stuff, but in the same thread
};

#endif
