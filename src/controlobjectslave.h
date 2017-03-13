#ifndef CONTROLOBJECTSLAVE_H
#define CONTROLOBJECTSLAVE_H

#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "control/control.h"
#include "configobject.h"

// This class is the successor of ControlObjectThread. It should be used for new
// code. It is better named and may save some CPU time because it is connected
// only on demand. There are many ControlObjectThread instances where the changed
// signal is not needed. This change will save the set() caller for doing
// unnecessary checks for possible connections.
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
    inline void emitValueChanged() {
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
            // broadcast as valueChanged() and not valueChangedByThis().
            m_pControl->reset();
        }
    }

  signals:
    // This signal must not connected by connect(). Use connectValueChanged()
    // instead. It will connect to the base ControlDoublePrivate as well.
    void valueChanged(double);

  protected slots:
    // Receives the value from the master control and re-emits either
    // valueChanged(double) or valueChangedByThis(double) based on pSetter.
    inline void slotValueChanged(double v, QObject* pSetter) {
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
