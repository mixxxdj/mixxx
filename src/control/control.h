#ifndef CONTROL_H
#define CONTROL_H

#include <QHash>
#include <QMutex>
#include <QString>
#include <QObject>
#include <QAtomicPointer>

#include "control/controlbehavior.h"
#include "control/controlvalue.h"
#include "configobject.h"

class ControlObject;

class ControlDoublePrivate : public QObject {
    Q_OBJECT
  public:
    virtual ~ControlDoublePrivate();

    // Used to implement control persistence. All controls that are marked
    // "persist in user config" get and set their value on creation/deletion
    // using this ConfigObject.
    static void setUserConfig(ConfigObject<ConfigValue>* pConfig) {
        s_pUserConfig = pConfig;
    }

    // Adds a ConfigKey for 'alias' to the control for 'key'. Can be used for
    // supporting a legacy / deprecated control. The 'key' control must exist
    // for this to work.
    static void insertAlias(const ConfigKey& alias, const ConfigKey& key);

    // Gets the ControlDoublePrivate matching the given ConfigKey. If pCreatorCO
    // is non-NULL, allocates a new ControlDoublePrivate for the ConfigKey if
    // one does not exist.
    static QSharedPointer<ControlDoublePrivate> getControl(
            const ConfigKey& key, bool warn = true,
            ControlObject* pCreatorCO = NULL, bool bIgnoreNops = true, bool bTrack = false,
            bool bPersist = false);

    // Adds all ControlDoublePrivate that currently exist to pControlList
    static void getControls(QList<QSharedPointer<ControlDoublePrivate> >* pControlsList);

    static QHash<ConfigKey, ConfigKey> getControlAliases();

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

    // Sets the control value.
    void set(double value, QObject* pSender);
    // directly sets the control value. Must be used from and only from the
    // ValueChangeRequest slot.
    void setAndConfirm(double value, QObject* pSender);
    // Gets the control value.
    inline double get() const {
        return m_value.getValue();
    }
    // Resets the control value to its default.
    void reset();

    // Set the behavior to be used when setting values and translating between
    // parameter and value space. Returns the previously set behavior (if any).
    // The caller must not delete the behavior at any time. The memory is managed
    // by this function.
    void setBehavior(ControlNumericBehavior* pBehavior);

    void setParameter(double dParam, QObject* pSender);
    double getParameter() const;
    double getParameterForValue(double value) const;
    double getParameterForMidiValue(double midiValue) const;

    void setMidiParameter(MidiOpCode opcode, double dParam);
    double getMidiParameter() const;

    inline bool ignoreNops() const {
        return m_bIgnoreNops;
    }

    inline void setDefaultValue(double dValue) {
        m_defaultValue.setValue(dValue);
    }

    inline double defaultValue() const {
        return m_defaultValue.getValue();
    }

    inline ControlObject* getCreatorCO() const {
        return m_pCreatorCO;
    }

    inline void removeCreatorCO() {
        m_pCreatorCO = NULL;
    }

    inline ConfigKey getKey() {
        return m_key;
    }

    // Connects a slot to the ValueChange request for CO validation. All change
    // requests issued by set are routed though the connected slot. This can
    // decide with its own thread safe solution if the requested value can be
    // confirmed by setAndConfirm() or not. Note: Once connected, the CO value
    // itself is ONLY set by setAndConfirm() typically called in the connected
    // slot.
    bool connectValueChangeRequest(const QObject* receiver,
                                   const char* method, Qt::ConnectionType type);

  signals:
    // Emitted when the ControlDoublePrivate value changes. pSender is a
    // pointer to the setter of the value (potentially NULL).
    void valueChanged(double value, QObject* pSender);
    void valueChangeRequest(double value);

  private:
    ControlDoublePrivate(ConfigKey key, ControlObject* pCreatorCO,
                         bool bIgnoreNops, bool bTrack, bool bPersist);
    void initialize();
    void setInner(double value, QObject* pSender);

    ConfigKey m_key;

    // Whether the control should persist in the Mixxx user configuration. The
    // value is loaded from configuration when the control is created and
    // written to the configuration when the control is deleted.
    bool m_bPersistInConfiguration;

    // User-visible, i18n name for what the control is.
    QString m_name;

    // User-visible, i18n descripton for what the control does.
    QString m_description;

    // Whether to ignore sets which would have no effect.
    bool m_bIgnoreNops;

    // Whether to track value changes with the stats framework.
    bool m_bTrack;
    QString m_trackKey;
    int m_trackType;
    int m_trackFlags;
    bool m_confirmRequired;

    // The control value.
    ControlValueAtomic<double> m_value;
    // The default control value.
    ControlValueAtomic<double> m_defaultValue;

    QSharedPointer<ControlNumericBehavior> m_pBehavior;

    ControlObject* m_pCreatorCO;

    // Hack to implement persistent controls. This is a pointer to the current
    // user configuration object (if one exists). In general, we do not want the
    // user configuration to be a singleton -- objects that need access to it
    // should be passed it explicitly. However, the Control system is so
    // pervasive that updating every control creation to include the
    // configuration object would be arduous.
    static ConfigObject<ConfigValue>* s_pUserConfig;

    // Hash of ControlDoublePrivate instantiations.
    static QHash<ConfigKey, QWeakPointer<ControlDoublePrivate> > s_qCOHash;
    // Hash of aliases between ConfigKeys. Solely used for looking up the first
    // alias associated with a key.
    static QHash<ConfigKey, ConfigKey> s_qCOAliasHash;

    // Mutex guarding access to s_qCOHash and s_qCOAliasHash.
    static QMutex s_qCOHashMutex;
};


#endif /* CONTROL_H */
