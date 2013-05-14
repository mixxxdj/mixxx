#ifndef CONTROL_H
#define CONTROL_H

#include <QHash>
#include <QMutex>
#include <QString>
#include <QObject>

#include "controlobjectbase.h"
#include "configobject.h"

class ControlNumericPrivate : public QObject {
    Q_OBJECT
  public:
    ControlNumericPrivate();
    ControlNumericPrivate(ConfigKey key, bool bIgnoreNops, bool bTrack);
    virtual ~ControlNumericPrivate();

    // Gets the ControlNumericPrivate matching the given ConfigKey. If bCreate
    // is true, allocates a new ControlNumericPrivate for the ConfigKey if one
    // does not exist.
    static ControlNumericPrivate* getControl(
        const ConfigKey& key,
        bool bCreate, bool bIgnoreNops=true, bool bTrack=false);
    static inline ControlNumericPrivate* getControl(
        const QString& group, const QString& item,
        bool bCreate, bool bIgnoreNops=true, bool bTrack=false) {
        ConfigKey key(group, item);
        return getControl(key, bCreate, bIgnoreNops, bTrack);
    }

    // Sets the control value.
    void set(const double& value, QObject* pSetter);
    // Gets the control value.
    double get() const;
    // Resets the control value to its default.
    void reset(QObject* pSetter);
    // Adds dValue to the control value.
    void add(double dValue, QObject* pSetter);
    // Subtracts dValue from the control value.
    void sub(double dValue, QObject* pSetter);

    inline bool ignoreNops() const {
        return m_bIgnoreNops;
    }

    inline void setDefaultValue(double dValue) {
        m_defaultValue.setValue(dValue);
    }
    inline double defaultValue() const {
        return m_defaultValue.getValue();
    }

  signals:
    // Emitted when the ControlNumericPrivate value changes. pSetter is a
    // pointer to the setter of the value (potentially NULL).
    void valueChanged(double value, QObject* pSetter);

  private:
    ConfigKey m_key;
    // Whether to ignore set/add/sub()'s which would have no effect.
    bool m_bIgnoreNops;

    // Whether to track value changes with the stats framework.
    bool m_bTrack;
    QString m_trackKey;
    int m_trackType;
    int m_trackFlags;

    // The control value.
    ControlObjectBase<double> m_value;
    // The default control value.
    ControlObjectBase<double> m_defaultValue;

    // Hash of ControlNumericPrivate instantiations.
    static QHash<ConfigKey,ControlNumericPrivate*> m_sqCOHash;
    // Mutex guarding access to the ControlNumericPrivate hash.
    static QMutex m_sqCOHashMutex;
};


#endif /* CONTROL_H */
