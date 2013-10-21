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

    // Gets the ControlDoublePrivate matching the given ConfigKey. If bCreate
    // is true, allocates a new ControlDoublePrivate for the ConfigKey if one
    // does not exist.
    static QSharedPointer<ControlDoublePrivate> getControl(
            const ConfigKey& key, bool warn = true,
            ControlObject* pCreatorCO = NULL, bool bIgnoreNops = true, bool bTrack = false);

    // Adds all ControlDoublePrivate that currently exist to pControlList
    static void getControls(QList<ControlDoublePrivate*>* pControlsList);

    // Sets the control value.
    void set(double value, QObject* pSender);
    // directly sets the control value. Must be used from and only from the
    // ValueChangeRequest slot.
    void setAndConfirm(double value, QObject* pSender);
    // Gets the control value.
    double get() const;
    // Resets the control value to its default.
    void reset();

    // Set the behavior to be used when setting values and translating between
    // parameter and value space. Returns the previously set behavior (if any).
    // The caller must nut delete the behavior at any time. The memory is managed
    // by this function.
    void setBehavior(ControlNumericBehavior* pBehavior);

    void setWidgetParameter(double dParam, QObject* pSender);
    double getWidgetParameter() const;

    void setMidiParameter(MidiOpCode opcode, double dParam);
    double getMidiParameter() const;

    inline bool ignoreNops() const {
        return m_bIgnoreNops;
    }

    inline void setDefaultValue(double dValue) {
        m_defaultValue.setValue(dValue);
    }

    inline double defaultValue() const {
        double default_value = m_defaultValue.getValue();
        return m_pBehavior ? m_pBehavior->defaultValue(default_value) : default_value;
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
    ControlDoublePrivate(ConfigKey key, ControlObject* pCreatorCO, bool bIgnoreNops, bool bTrack);
    void initialize();
    void setInner(double value, QObject* pSender);

    ConfigKey m_key;
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

    // Hash of ControlDoublePrivate instantiations.
    static QHash<ConfigKey, QWeakPointer<ControlDoublePrivate> > m_sqCOHash;
    // Mutex guarding access to the ControlDoublePrivate hash.
    static QMutex m_sqCOHashMutex;
};


#endif /* CONTROL_H */
