#pragma once

#include <QElapsedTimer>

#include "controllers/controllermappinginfo.h"
#include "util/duration.h"
#include "util/runtimeloggingcategory.h"

class ControllerJSProxy;
class ControllerScriptEngineLegacy;

enum class PhysicalTransportProtocol {
    UNKNOWN,
    USB,
    BlueTooth,
    I2C,
    SPI,
    FireWire // IEEE 1394
};

enum class DataRepresentationProtocol {
    MIDI,
    HID,
    USB_BULK_TRANSFER // Bulk is only one of the 4 USB transfer modes
};

/// This is a base class representing a physical (or software) controller.  It
/// must be inherited by a class that implements it on some API. Note that the
/// subclass' destructor should call close() at a minimum.
class Controller : public QObject {
    Q_OBJECT
  public:
    explicit Controller(const QString& deviceName);
    ~Controller() override;  // Subclass should call close() at minimum.

    /// The object that is exposed to the JS scripts as the "controller" object.
    /// Subclasses of Controller can return a subclass of ControllerJSProxy to further
    /// customize their JS api.
    virtual ControllerJSProxy* jsProxy();

    /// Returns the extension for the controller (type) mapping files.  This is
    /// used by the ControllerManager to display only relevant mapping files for
    /// the controller (type.)
    virtual QString mappingExtension() = 0;

    virtual std::shared_ptr<LegacyControllerMapping> cloneMapping() = 0;
    /// WARNING: LegacyControllerMapping is not thread safe!
    /// Clone the mapping before passing to setMapping for use in the controller polling thread.
    virtual void setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) = 0;

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
    virtual PhysicalTransportProtocol getPhysicalTransportProtocol() const = 0;
    static QString physicalTransport2String(PhysicalTransportProtocol protocol);
    virtual DataRepresentationProtocol getDataRepresentationProtocol() const = 0;

    virtual QString getVendorString() const = 0;
    virtual QString getProductString() const = 0;
    virtual std::optional<uint16_t> getVendorId() const = 0;
    virtual std::optional<uint16_t> getProductId() const = 0;
    virtual QString getSerialNumber() const = 0;
    virtual std::optional<uint8_t> getUsbInterfaceNumber() const = 0;

    virtual bool isMappable() const = 0;
    inline bool isLearning() const {
        return m_bLearning;
    }

    inline std::shared_ptr<ControllerScriptEngineLegacy> getScriptEngine() const {
        return m_pScriptEngineLegacy;
    }

    virtual bool matchMapping(const MappingInfo& mapping) = 0;

  signals:
    /// Emitted when the controller is opened or closed.
    void openChanged(bool bOpen);

    /// Emitted when the controller has started a new engine.
    void engineStarted(const ControllerScriptEngineLegacy* engine);

    /// Emitted when the controller has stopped its engine.
    void engineStopped();

    // Making these slots protected/private ensures that other parts of Mixxx can
    // only signal them which allows us to use no locks.
  protected slots:
    // TODO(XXX) move this into the inherited classes since is not called here
    // (via Controller) and re-implemented anyway in most cases.

    // Handles packets of raw bytes and passes them to an ".incomingData" script
    // function that is assumed to exist. (Sub-classes may want to reimplement
    // this if they have an alternate way of handling such data.)
    virtual void receive(const QByteArray& data, mixxx::Duration timestamp);

    virtual bool applyMapping(const QString& resourcePath);
    virtual void slotBeforeEngineShutdown();

    // Puts the controller in and out of learning mode.
    void startLearning();
    void stopLearning();

  protected:
    template<typename SpecificMappingType>
        requires(std::is_final_v<SpecificMappingType> == true)
    std::unique_ptr<SpecificMappingType> downcastAndClone(const LegacyControllerMapping* pMapping) {
        // When unsetting a mapping (select 'No mapping') we receive a nullptr
        if (!pMapping) {
            return nullptr;
        }
        auto* pSpecifiedMapping = dynamic_cast<const SpecificMappingType*>(pMapping);
        VERIFY_OR_DEBUG_ASSERT(pSpecifiedMapping) {
            return nullptr;
        }
        // Controller cannot take ownership if pMapping is referenced elsewhere because
        // the controller polling thread needs exclusive accesses to the non-thread safe
        // LegacyControllerMapping. So we do a deep copy here.
        return std::make_unique<SpecificMappingType>(*pSpecifiedMapping);
    }

    // The length parameter is here for backwards compatibility for when scripts
    // were required to specify it.
    virtual void send(const QList<int>& data, unsigned int length = 0);

    // To be called in sub-class' open() functions after opening the device but
    // before starting any input polling/processing.
    virtual void startEngine();

    // To be called in sub-class' close() functions after stopping any input
    // polling/processing but before closing the device.
    virtual void stopEngine();

    // To be called when receiving events
    void triggerActivity();

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

    const QString m_sDeviceName;
    const RuntimeLoggingCategory m_logBase;
    const RuntimeLoggingCategory m_logInput;
    const RuntimeLoggingCategory m_logOutput;

  public:
    // This must be reimplemented by sub-classes desiring to send raw bytes to a
    // controller. Return true in case of successful completion, false in case
    // of partial completion or failure.
    virtual bool sendBytes(const QByteArray& data) = 0;

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
    /// Controllers have multiple ownership over an engine.
    /// A ControllerScriptEngine must not own a controller.
    std::shared_ptr<ControllerScriptEngineLegacy> m_pScriptEngineLegacy;

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
    friend class MidiControllerTest;
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
