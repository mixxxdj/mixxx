#pragma once

#include <QMutex>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QHash>

#include "preferences/usersettings.h"
#include "control/control.h"

class ControlString;

// ControlStringPrivate is the thread-safe implementation backing ControlString
// Similar to ControlDoublePrivate but for UTF-8 strings
class ControlStringPrivate : public QObject {
    Q_OBJECT
  public:
    ~ControlStringPrivate() override;

    // Used to implement control persistence. All controls that are marked
    // "persist in user config" get and set their value on creation/deletion
    // using this UserSettings.
    static void setUserConfig(const UserSettingsPointer& pConfig);

    // Gets the ControlStringPrivate matching the given ConfigKey. If pCreatorCO
    // is non-NULL, allocates a new ControlStringPrivate for the ConfigKey if
    // one does not exist.
    static QSharedPointer<ControlStringPrivate> getControl(
            const ConfigKey& key,
            ControlFlags flags = ControlFlag::None,
            ControlString* pCreatorCO = nullptr,
            bool bIgnoreNops = true,
            bool bPersist = false,
            const QString& defaultValue = QString());

    static QSharedPointer<ControlStringPrivate> getDefaultControl();

    // Returns a list of all existing instances.
    static QList<QSharedPointer<ControlStringPrivate>> getAllInstances();
    // Clears all existing instances and returns them as a list.
    static QList<QSharedPointer<ControlStringPrivate>> takeAllInstances();

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
    void set(const QString& value, QObject* pSender);
    // directly sets the control value. Must be used from and only from the
    // ValueChangeRequest slot.
    void setAndConfirm(const QString& value, QObject* pSender);
    // Gets the control value.
    QString get() const;
    // Resets the control value to its default.
    void reset();

    bool ignoreNops() const {
        return m_bIgnoreNops;
    }

    void setDefaultValue(const QString& value);
    QString defaultValue() const;

    ControlString* getCreatorCO() const;
    bool resetCreatorCO(ControlString* pCreatorCO);
    void deleteCreatorCO();

    const ConfigKey& getKey() const {
        return m_key;
    }

    // Connects a slot to the ValueChange request for CO validation.
    template <typename Receiver, typename Slot>
    bool connectValueChangeRequest(Receiver receiver, Slot func,
                                   Qt::ConnectionType type = Qt::AutoConnection) {
        return connect(this, &ControlStringPrivate::valueChangeRequest,
                       receiver, func, type);
    }

  signals:
    void valueChanged(const QString& value, QObject* pSetter);
    void valueChangeRequest(const QString& value);

  private slots:
    void slotValueChangeRequest(const QString& value);

  private:
    ControlStringPrivate(const ConfigKey& key,
                         ControlString* pCreatorCO,
                         bool bIgnoreNops,
                         bool bPersist,
                         const QString& defaultValue);

    void initialize(const QString& defaultValue);

    /// Global map of ConfigKey to ControlStringPrivate instances
    static QHash<ConfigKey, QWeakPointer<ControlStringPrivate>> s_controlHash;
    static QMutex s_controlHashMutex;
    static UserSettingsPointer s_pUserConfig;
    static QSharedPointer<ControlStringPrivate> s_pDefaultControl;

    ConfigKey m_key;
    QString m_name;
    QString m_description;

    mutable QMutex m_mutex;
    QString m_value;
    QString m_defaultValue;
    bool m_bIgnoreNops;
    bool m_bPersist;
    
    QAtomicPointer<ControlString> m_pCreatorCO;

    friend class ControlString;
};

class ControlString : public QObject {
    Q_OBJECT
  public:
    ControlString();

    // bIgnoreNops: Don't emit a signal if the CO is set to its current value.
    // bPersist: Store value on exit, load on startup.
    // defaultValue: default value of CO. If CO is persistent and there is no valid
    //               value found in the config, this is also the initial value.
    ControlString(const ConfigKey& key,
                  bool bIgnoreNops = true,
                  bool bPersist = false,
                  const QString& defaultValue = QString());
    virtual ~ControlString();

    // Returns a pointer to the ControlString matching the given ConfigKey
    static ControlString* getControl(const ConfigKey& key, ControlFlags flags = ControlFlag::None);
    static ControlString* getControl(const QString& group,
                                     const QString& item,
                                     ControlFlags flags = ControlFlag::None) {
        ConfigKey key(group, item);
        return getControl(key, flags);
    }

    // Checks whether a ControlString exists or not
    static bool exists(const ConfigKey& key);

    QString name() const {
        return m_pControl ? m_pControl->name() : QString();
    }

    void setName(const QString& name) {
        if (m_pControl) {
            m_pControl->setName(name);
        }
    }

    const QString description() const {
        return m_pControl ? m_pControl->description() : QString();
    }

    void setDescription(const QString& description) {
        if (m_pControl) {
            m_pControl->setDescription(description);
        }
    }

    // Return the key of the object
    inline ConfigKey getKey() const {
        return m_key;
    }

    // Returns the value of the ControlString
    inline QString get() const {
        return m_pControl ? m_pControl->get() : QString();
    }

    // Instantly returns the value of the ControlString
    static QString get(const ConfigKey& key);

    // Sets the ControlString value. May require confirmation by owner.
    inline void set(const QString& value) {
        if (m_pControl) {
            m_pControl->set(value, this);
        }
    }

    // Sets the ControlString value and confirms it.
    inline void setAndConfirm(const QString& value) {
        if (m_pControl) {
            m_pControl->setAndConfirm(value, this);
        }
    }

    // Forces the control to 'value', regardless of whether it has a change
    // request handler attached (identical to setAndConfirm).
    inline void forceSet(const QString& value) {
        setAndConfirm(value);
    }

    // Instantly sets the value of the ControlString
    static void set(const ConfigKey& key, const QString& value);

    // Sets the default value
    inline void reset() {
        if (m_pControl) {
            m_pControl->reset();
        }
    }

    inline void setDefaultValue(const QString& value) {
        if (m_pControl) {
            m_pControl->setDefaultValue(value);
        }
    }
    inline QString defaultValue() const {
        return m_pControl ? m_pControl->defaultValue() : QString();
    }

    // Connects a Qt slot to a signal that is delivered when a new value change
    // request arrives for this control.
    template <typename Receiver, typename Slot>
    bool connectValueChangeRequest(Receiver receiver, Slot func,
                                   Qt::ConnectionType type = Qt::AutoConnection) {
        bool ret = false;
        if (m_pControl) {
            ret = m_pControl->connectValueChangeRequest(receiver, func, type);
        }
        return ret;
    }

    // Installs a value-change request handler that ignores all sets.
    void setReadOnly();

  signals:
    void valueChanged(const QString& value);

  protected:
    // Key of the object
    ConfigKey m_key;
    QSharedPointer<ControlStringPrivate> m_pControl;

  private slots:
    void privateValueChanged(const QString& value, QObject* pSetter);
    void readOnlyHandler(const QString& v);

  private:
    ControlString(ControlString&&) = delete;
    ControlString(const ControlString&) = delete;
    ControlString& operator=(ControlString&&) = delete;
    ControlString& operator=(const ControlString&) = delete;

    inline bool ignoreNops() const {
        return m_pControl ? m_pControl->ignoreNops() : true;
    }
};
