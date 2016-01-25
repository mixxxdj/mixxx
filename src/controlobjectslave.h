#ifndef CONTROLOBJECTSLAVE_H
#define CONTROLOBJECTSLAVE_H

#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "control/control.h"
#include "preferences/usersettings.h"

// This class is the successor of ControlObjectThread. It should be used for
// new code to avoid unnecessary locking during send if no slot is connected.
// Do not (re-)connect slots during runtime, since this locks the mutex in
// QMetaObject::activate().
// Be sure that the ControlObjectSlave is created and deleted from the same
// thread, otherwise a pending signal may lead to a segfault (Bug #1406124).
// Parent it to the the creating object to achieve this.
class ControlObjectSlave : public QObject {
    Q_OBJECT
  public:
    ControlObjectSlave(QObject* pParent = NULL);
    ControlObjectSlave(const QString& g, const QString& i, QObject* pParent = NULL);
    ControlObjectSlave(const char* g, const char* i, QObject* pParent = NULL);
    ControlObjectSlave(const ConfigKey& key, QObject* pParent = NULL);
    virtual ~ControlObjectSlave();

    void initialize(const ConfigKey& key);

    const ConfigKey& getKey() const {
        return m_key;
    }

    bool connectValueChanged(const QObject* receiver,
            const char* method, Qt::ConnectionType type = Qt::AutoConnection);
    bool connectValueChanged(
            const char* method, Qt::ConnectionType type = Qt::AutoConnection);

    // Called from update();
    virtual void emitValueChanged() {
        emit(valueChanged(get()));
    }

    inline bool valid() const { return m_pControl != NULL; }

    // Returns the value of the object. Thread safe, non-blocking.
    inline double get() const {
        return m_pControl ? m_pControl->get() : 0.0;
    }

    // Returns the bool interpretation of the value
    inline bool toBool() const {
        return get() > 0.0;
    }

    // Returns the parameterized value of the object. Thread safe, non-blocking.
    inline double getParameter() const {
        return m_pControl ? m_pControl->getParameter() : 0.0;
    }

    // Returns the parameterized value of the object. Thread safe, non-blocking.
    inline double getParameterForValue(double value) const {
        return m_pControl ? m_pControl->getParameterForValue(value) : 0.0;
    }

    // Returns the normalized parameter of the object. Thread safe, non-blocking.
    inline double getDefault() const {
        return m_pControl ? m_pControl->defaultValue() : 0.0;
    }

  public slots:
    // Set the control to a new value. Non-blocking.
    inline void slotSet(double v) {
        set(v);
    }
    // Sets the control value to v. Thread safe, non-blocking.
    void set(double v) {
        if (m_pControl) {
            m_pControl->set(v, this);
        }
    }
    // Sets the control parameterized value to v. Thread safe, non-blocking.
    void setParameter(double v) {
        if (m_pControl) {
            m_pControl->setParameter(v, this);
        }
    }
    // Resets the control to its default value. Thread safe, non-blocking.
    void reset() {
        if (m_pControl) {
            // NOTE(rryan): This is important. The originator of this action does
            // not know the resulting value so it makes sense that we should emit a
            // general valueChanged() signal even though the change originated from
            // us. For this reason, we provide NULL here so that the change is
            // not filtered in valueChanged()
            m_pControl->reset();
        }
    }

  signals:
    // This signal must not connected by connect(). Use connectValueChanged()
    // instead. It will connect to the base ControlDoublePrivate as well.
    void valueChanged(double);

  protected slots:
    // Receives the value from the master control by a unique direct connection
    void slotValueChangedDirect(double v, QObject* pSetter) {
        if (pSetter != this) {
            // This is base implementation of this function without scaling
            emit(valueChanged(v));
        }
    }

    // Receives the value from the master control by a unique auto connection
    void slotValueChangedAuto(double v, QObject* pSetter) {
        if (pSetter != this) {
            // This is base implementation of this function without scaling
            emit(valueChanged(v));
        }
    }

    // Receives the value from the master control by a unique Queued connection
    void slotValueChangedQueued(double v, QObject* pSetter) {
        if (pSetter != this) {
            // This is base implementation of this function without scaling
            emit(valueChanged(v));
        }
    }

  protected:
    ConfigKey m_key;
    // Pointer to connected control.
    QSharedPointer<ControlDoublePrivate> m_pControl;
};

#endif // CONTROLOBJECTSLAVE_H
