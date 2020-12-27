#pragma once

#include <QElapsedTimer>
#include <QTimerEvent>

#include "controllers/controllermappinginfo.h"
#include "controllers/controllermappingvisitor.h"
#include "controllers/controllervisitor.h"
#include "controllers/legacycontrollermapping.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "util/duration.h"

class ControllerJSProxy;

/// This is a base class representing a physical (or software) controller.  It
/// must be inherited by a class that implements it on some API. Note that the
/// subclass' destructor should call close() at a minimum.
class Controller : public QObject, ConstLegacyControllerMappingVisitor {
    Q_OBJECT
  public:
    explicit Controller();
    ~Controller() override;  // Subclass should call close() at minimum.

    /// The object that is exposed to the JS scripts as the "controller" object.
    /// Subclasses of Controller can return a subclass of ControllerJSProxy to further
    /// customize their JS api.
    virtual ControllerJSProxy* jsProxy();

    /// Returns the extension for the controller (type) mapping files.  This is
    /// used by the ControllerManager to display only relevant mapping files for
    /// the controller (type.)
    virtual QString mappingExtension() = 0;

    void setMapping(const LegacyControllerMapping& mapping) {
        // We don't know the specific type of the mapping so we need to ask
        // the mapping to call our visitor methods with its type.
        mapping.accept(this);
    }

    virtual void accept(ControllerVisitor* visitor) = 0;

    // Returns a clone of the Controller's loaded mapping.
    virtual LegacyControllerMappingPointer getMapping() const = 0;

    inline bool isOpen() const {
        return m_bIsOpen;
    }
    inline bool isOutputDevice() const {
        return m_bIsOutputDevice;
    }
    inline bool isInputDevice() const {
        return m_bIsInputDevice;
    }
    inline const QString& getName() const {
        return m_sDeviceName;
    }
    inline const QString& getCategory() const {
        return m_sDeviceCategory;
    }
    virtual bool isMappable() const = 0;
    inline bool isLearning() const {
        return m_bLearning;
    }

    virtual bool matchMapping(const MappingInfo& mapping) = 0;

  signals:
    // Emitted when a new mapping is loaded. pMapping is a /clone/ of the loaded
    // mapping, not a pointer to the mapping itself.
    void mappingLoaded(LegacyControllerMappingPointer pMapping);

    /// Emitted when the controller is opened or closed.
    void openChanged(bool bOpen);

    // Making these slots protected/private ensures that other parts of Mixxx can
    // only signal them which allows us to use no locks.
  protected slots:
    // TODO(XXX) move this into the inherited classes since is not called here
    // (via Controller) and re-implemented anyway in most cases.

    // Handles packets of raw bytes and passes them to an ".incomingData" script
    // function that is assumed to exist. (Sub-classes may want to reimplement
    // this if they have an alternate way of handling such data.)
    virtual void receive(const QByteArray& data, mixxx::Duration timestamp);

    virtual bool applyMapping();

    // Puts the controller in and out of learning mode.
    void startLearning();
    void stopLearning();

  protected:
    // The length parameter is here for backwards compatibility for when scripts
    // were required to specify it.
    virtual void send(const QList<int>& data, unsigned int length = 0);

    // This must be reimplemented by sub-classes desiring to send raw bytes to a
    // controller.
    virtual void sendBytes(const QByteArray& data) = 0;

    // To be called in sub-class' open() functions after opening the device but
    // before starting any input polling/processing.
    virtual void startEngine();

    // To be called in sub-class' close() functions after stopping any input
    // polling/processing but before closing the device.
    virtual void stopEngine();

    // To be called when receiving events
    void triggerActivity();

    inline ControllerScriptEngineLegacy* getScriptEngine() const {
        return m_pScriptEngineLegacy;
    }
    inline void setDeviceName(const QString& deviceName) {
        m_sDeviceName = deviceName;
    }
    inline void setDeviceCategory(const QString& deviceCategory) {
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
        emit openChanged(m_bIsOpen);
    }

  private: // but used by ControllerManager

    virtual int open() = 0;
    virtual int close() = 0;
    // Requests that the device poll if it is a polling device. Returns true
    // if events were handled.
    virtual bool poll() { return false; }

    // Returns true if this device should receive polling signals via calls to
    // its poll() method.
    virtual bool isPolling() const {
        return false;
    }

  private:
    // Returns a pointer to the currently loaded controller mapping. For internal
    // use only.
    virtual LegacyControllerMapping* mapping() = 0;
    ControllerScriptEngineLegacy* m_pScriptEngineLegacy;

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
    bool m_bLearning;
    QElapsedTimer m_userActivityInhibitTimer;

    friend class ControllerJSProxy;
    // accesses lots of our stuff, but in the same thread
    friend class ControllerManager;
    // For testing
    friend class LegacyControllerMappingValidationTest;
};

// An object of this class gets exposed to the JS engine, so the methods of this class
// constitute the api that is provided to scripts under "controller" object.
// See comments on ControllerEngineJSProxy.
class ControllerJSProxy : public QObject {
    Q_OBJECT
  public:
    explicit ControllerJSProxy(Controller* m_pController)
            : m_pController(m_pController) {
    }

    // The length parameter is here for backwards compatibility for when scripts
    // were required to specify it.
    Q_INVOKABLE virtual void send(const QList<int>& data, unsigned int length = 0) {
        Q_UNUSED(length);
        m_pController->send(data, data.length());
    }

  private:
    Controller* const m_pController;
};
