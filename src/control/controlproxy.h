#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "control/control.h"
#include "preferences/usersettings.h"

//// This class is the successor of ControlObjectThread. It should be used for
/// new code to avoid unnecessary locking during send if no slot is connected.
/// Do not (re-)connect slots during runtime, since this locks the mutex in
/// QMetaObject::activate().
/// Be sure that the ControlProxy is created and deleted from the same
/// thread, otherwise a pending signal may lead to a segfault (Issue #7773).
/// Parent it to the the creating object to achieve this.
class ControlProxy : public QObject {
    Q_OBJECT
  public:
    ControlProxy(const QString& g,
            const QString& i,
            QObject* pParent = nullptr,
            ControlFlags flags = ControlFlag::None);
    ControlProxy(const ConfigKey& key,
            QObject* pParent = nullptr,
            ControlFlags flags = ControlFlag::None);
    ~ControlProxy() override;

    const ConfigKey& getKey() const;

    template<typename Receiver, typename Slot>
    bool connectValueChanged(Receiver receiver,
            Slot func,
            Qt::ConnectionType requestedConnectionType = Qt::AutoConnection) {
        if (!valid()) {
            return false;
        }

        // We connect to the
        // ControlObjectPrivate only once and in a way that
        // the requested ConnectionType is working as desired.
        // We try to avoid direct connections if not requested
        // since you cannot safely delete an object with a pending
        // direct connection. This fixes issue #7773
        // requested: Auto -> COP = Auto / SCO = Auto
        // requested: Direct -> COP = Direct / SCO = Direct
        // requested: Queued -> COP = Queued / SCO = Auto
        // requested: BlockingQueued -> Assert(false)

        Qt::ConnectionType scoConnection;
        switch (requestedConnectionType) {
        case Qt::AutoConnection:
        case Qt::QueuedConnection:
            scoConnection = Qt::AutoConnection;
            break;
        case Qt::DirectConnection:
            scoConnection = Qt::DirectConnection;
            break;
        case Qt::BlockingQueuedConnection:
            // We must not block the signal source by a blocking connection
            [[fallthrough]];
        default:
            DEBUG_ASSERT(false);
            return false;
        }

        if (!connect(this, &ControlProxy::valueChanged, receiver, func, scoConnection)) {
            return false;
        }

        // Connect to ControlObjectPrivate only if required. Do not allow
        // duplicate connections.

        // use only explicit direct connection if requested
        // the caller must not delete this until the all signals are
        // processed to avoid segfaults
        Qt::ConnectionType copConnection = static_cast<Qt::ConnectionType>(
                requestedConnectionType | Qt::UniqueConnection);

        // clazy requires us to to pass a member function to connect() directly
        // (i.e. w/o and intermediate variable) when used with
        // Qt::UniqueConnection. Otherwise it detects a false positive and
        // throws a [-Wclazy-lambda-unique-connection] warning.
        switch (requestedConnectionType) {
        case Qt::AutoConnection:
            connect(m_pControl.data(), &ControlDoublePrivate::valueChanged, this, &ControlProxy::slotValueChangedAuto, copConnection);
            break;
        case Qt::DirectConnection:
            connect(m_pControl.data(), &ControlDoublePrivate::valueChanged, this, &ControlProxy::slotValueChangedDirect, copConnection);
            break;
        case Qt::QueuedConnection:
            connect(m_pControl.data(), &ControlDoublePrivate::valueChanged, this, &ControlProxy::slotValueChangedQueued, copConnection);
            break;
        default:
            // Should be unreachable, but just to make sure ;-)
            DEBUG_ASSERT(false);
            return false;
        }
        return true;
    }

    /// Called from update();
    virtual void emitValueChanged() {
        emit valueChanged(get());
    }

    inline bool valid() const {
        return m_pControl->getKey().isValid();
    }

    /// Returns the value of the object. Thread safe, non-blocking.
    inline double get() const {
        return m_pControl->get();
    }

    /// Returns the bool interpretation of the value. Thread safe, non-blocking.
    inline bool toBool() const {
        return get() > 0.0;
    }

    /// Returns the parameterized value of the object. Thread safe, non-blocking.
    inline double getParameter() const {
        return m_pControl->getParameter();
    }

    /// Returns the parameterized value of the object. Thread safe, non-blocking.
    inline double getParameterForValue(double value) const {
        return m_pControl->getParameterForValue(value);
    }

    /// Returns the normalized parameter of the object. Thread safe, non-blocking.
    inline double getDefault() const {
        return m_pControl->defaultValue();
    }

  public slots:
    /// Sets the control value to v. Thread safe, non-blocking.
    void set(double v) {
        m_pControl->set(v, this);
    }
    /// Sets the control parameterized value to v. Thread safe, non-blocking.
    void setParameter(double v) {
        m_pControl->setParameter(v, this);
    }
    /// Resets the control to its default value. Thread safe, non-blocking.
    void reset() {
        // NOTE(rryan): This is important. The originator of this action does
        // not know the resulting value so it makes sense that we should emit a
        // general valueChanged() signal even though the change originated from
        // us. For this reason, we provide NULL here so that the change is
        // not filtered in valueChanged()
        m_pControl->reset();
    }

  signals:
    /// This signal must not connected by connect(). Use connectValueChanged()
    /// instead. It will connect to the base ControlDoublePrivate as well.
    void valueChanged(double);

  protected slots:
    /// Receives the value from the primary control by a unique direct connection
    void slotValueChangedDirect(double v, QObject* pSetter) {
        if (pSetter != this) {
            // This is base implementation of this function without scaling
            emit valueChanged(v);
        }
    }

    /// Receives the value from the primary control by a unique auto connection
    void slotValueChangedAuto(double v, QObject* pSetter) {
        if (pSetter != this) {
            // This is base implementation of this function without scaling
            emit valueChanged(v);
        }
    }

    /// Receives the value from the primary control by a unique Queued connection
    void slotValueChangedQueued(double v, QObject* pSetter) {
        if (pSetter != this) {
            // This is base implementation of this function without scaling
            emit valueChanged(v);
        }
    }

  protected:
    /// Pointer to connected control.
    QSharedPointer<ControlDoublePrivate> m_pControl;
};
