#pragma once

#include <QAtomicPointer>
#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "control/controlbehavior.h"
#include "control/controlvalue.h"
#include "preferences/usersettings.h"
#include "util/stat.h"

class ControlObject;

enum class ControlFlag {
    None = 0,
    /// Do not throw an assertion if the key is invalid. Needed for controller
    /// mappings and skins.
    AllowInvalidKey = 1,
    /// Don't throw an assertion when trying to access a non-existing CO.
    /// Needed for controller mappings and skins.
    NoAssertIfMissing = 1 << 1,
    /// Don't log a warning when trying to access a non-existing CO.
    NoWarnIfMissing = (1 << 2) | NoAssertIfMissing,
    AllowMissingOrInvalid = AllowInvalidKey | NoAssertIfMissing,
};

Q_DECLARE_FLAGS(ControlFlags, ControlFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(ControlFlags)

// note that this could fit into a std::uint8_t, but when using this with QFlags,
// the resulting type has sizeof(int) anyways. See QTBUG-128105
enum class ControlConfigFlag {
    None = 0,
    // Whether to ignore sets which would have no effect.
    IgnoreNops = 1,
    // Whether to track value changes with the stats framework.
    Track = 1 << 1,
    // Whether the control should persist in the Mixxx user configuration. The
    // value is loaded from configuration when the control is created and
    // written to the configuration when the control is deleted.
    Persist = 1 << 2,
    // Whether this control will be issued repeatedly if the keyboard key is held.
    KeyboardRepeatable = 1 << 3,

    // Default configuration as used commonly throughout mixxx (may be subject to change)
    Default = IgnoreNops,
};

Q_DECLARE_FLAGS(ControlConfigFlags, ControlConfigFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(ControlConfigFlags)

class ControlDoublePrivate : public QObject {
    Q_OBJECT
  public:
    ~ControlDoublePrivate() override;

    // Used to implement control persistence. All controls that are marked
    // "persist in user config" get and set their value on creation/deletion
    // using this UserSettings.
    static void setUserConfig(const UserSettingsPointer& pConfig);

    // Adds a ConfigKey for 'alias' to the control for 'key'. Can be used for
    // supporting a legacy / deprecated control. The 'key' control must exist
    // for this to work.
    static void insertAlias(const ConfigKey& alias, const ConfigKey& key);

    // Gets the ControlDoublePrivate matching the given ConfigKey. If pCreatorCO
    // is non-NULL, allocates a new ControlDoublePrivate for the ConfigKey if
    // one does not exist.
    static QSharedPointer<ControlDoublePrivate> getControl(
            const ConfigKey& key,
            ControlFlags flags = ControlFlag::None,
            ControlObject* pCreatorCO = nullptr,
            ControlConfigFlags configFlags = ControlConfigFlag::Default,
            double defaultValue = 0.0);
    static QSharedPointer<ControlDoublePrivate> getDefaultControl();

    // Returns a list of all existing instances.
    static QList<QSharedPointer<ControlDoublePrivate>> getAllInstances();
    // Clears all existing instances and returns them as a list.
    static QList<QSharedPointer<ControlDoublePrivate>> takeAllInstances();

    static QHash<ConfigKey, ConfigKey> getControlAliases();

    // used as a transitional tool from the boolean-based APIs to the
    // QFlag-based one.
    constexpr static ControlConfigFlags configFlagFromBools(
            bool bIgnoreNops,
            bool bTrack,
            bool bPersist) {
        using enum ControlConfigFlag;
        return ControlConfigFlags()
                .setFlag(IgnoreNops, bIgnoreNops)
                .setFlag(Track, bTrack)
                .setFlag(Persist, bPersist);
    };

    const QString& name() const {
        return m_name;
    }

    void setName(const QString& name) {
        m_name = name;
    }

    const QString& description() const {
        return m_description;
    }

    void setDescription(const QString& description) {
        m_description = description;
    }

    void setKbdRepeatable(bool enable) {
        m_configFlags.setFlag(ControlConfigFlag::KeyboardRepeatable, enable);
    }

    bool getKbdRepeatable() const {
        return m_configFlags.testFlag(ControlConfigFlag::KeyboardRepeatable);
    }

    // Sets the control value.
    void set(double value, QObject* pSender);
    // directly sets the control value. Must be used from and only from the
    // ValueChangeRequest slot.
    void setAndConfirm(double value, QObject* pSender);
    // Gets the control value.
    double get() const {
        return m_value.getValue();
    }
    // Resets the control value to its default.
    void reset();

    // Set the behavior to be used when setting values and translating between
    // parameter and value space. Returns the previously set behavior (if any).
    // Callers must allocate the passed behavior using new and ownership to this
    // memory is passed with the function call!!
    // TODO: Pass a std::unique_ptr instead of a plain pointer to ensure this
    // transfer of ownership.
    void setBehavior(ControlNumericBehavior* pBehavior);

    void setParameter(double dParam, QObject* pSender);
    double getParameter() const;
    double getParameterForValue(double value) const;
    double getParameterForMidi(double midiValue) const;

    void setValueFromMidi(MidiOpCode opcode, double dParam);
    double getMidiParameter() const;

    bool ignoreNops() const {
        return m_configFlags.testFlag(ControlConfigFlag::IgnoreNops);
    }

    void setDefaultValue(double dValue) {
        m_defaultValue.setValue(dValue);
    }

    double defaultValue() const {
        return m_defaultValue.getValue();
    }

    ControlObject* getCreatorCO() const {
        return m_pCreatorCO.loadAcquire();
    }

    bool resetCreatorCO(ControlObject* pCreatorCO) {
        return m_pCreatorCO.testAndSetOrdered(pCreatorCO, nullptr);
    }
    void deleteCreatorCO();

    const ConfigKey& getKey() {
        return m_key;
    }

    // Connects a slot to the ValueChange request for CO validation. All change
    // requests issued by set are routed though the connected slot. This can
    // decide with its own thread safe solution if the requested value can be
    // confirmed by setAndConfirm() or not. Note: Once connected, the CO value
    // itself is ONLY set by setAndConfirm() typically called in the connected
    // slot.
    template <typename Receiver, typename Slot>
    bool connectValueChangeRequest(Receiver receiver,
                                   Slot func, Qt::ConnectionType type) {
        // confirmation is only required if connect was successful
        m_confirmRequired = connect(this, &ControlDoublePrivate::valueChangeRequest,
                    receiver, func, type);
        return m_confirmRequired;
    }

  signals:
    // Emitted when the ControlDoublePrivate value changes. pSender is a
    // pointer to the setter of the value (potentially NULL).
    void valueChanged(double value, QObject* pSender);
    void valueChangeRequest(double value);

  protected:
    ControlDoublePrivate();

  private:
    ControlDoublePrivate(
            const ConfigKey& key,
            ControlObject* pCreatorCO,
            ControlConfigFlags configFlags,
            double defaultValue);
    ControlDoublePrivate(ControlDoublePrivate&&) = delete;
    ControlDoublePrivate(const ControlDoublePrivate&) = delete;
    ControlDoublePrivate& operator=(ControlDoublePrivate&&) = delete;
    ControlDoublePrivate& operator=(const ControlDoublePrivate&) = delete;

    void initialize(double defaultValue);
    virtual void setInner(double value, QObject* pSender);

    const ConfigKey m_key;

    QSharedPointer<ControlNumericBehavior> m_pBehavior;

    // User-visible, i18n name for what the control is.
    QString m_name;

    // User-visible, i18n description for what the control does.
    QString m_description;

    // The control value.
    ControlValueAtomic<double> m_value;
    // The default control value.
    ControlValueAtomic<double> m_defaultValue;

    QAtomicPointer<ControlObject> m_pCreatorCO;

    QString m_trackKey;

    // Note: keep the order of the members below to not introduce gaps due to
    // memory alignment in this often used class. Whether to track value changes
    // with the stats framework.
    Stat::StatType m_trackType;
    Stat::ComputeFlags m_trackFlags;
    ControlConfigFlags m_configFlags;
    bool m_confirmRequired;

};

/// The constant ControlDoublePrivate version is used as dummy for default
/// constructed control objects
class ControlDoublePrivateConst : public ControlDoublePrivate {
    Q_OBJECT
  public:
    ~ControlDoublePrivateConst() override = default;

    void setInner(double value, QObject* pSender) override {
        Q_UNUSED(value)
        Q_UNUSED(pSender)
        DEBUG_ASSERT(!"Trying to modify a default constructed (const) control object");
    };

  protected:
    ControlDoublePrivateConst() = default;

    friend ControlDoublePrivate;
};
