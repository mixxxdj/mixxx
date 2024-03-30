#pragma once

#include <QSharedPointer>
#include <QString>

#include "control/control.h"

/// This is the light version of a control proxy without the QObject overhead.
/// This should be used when no signal connections are used.
/// It is basically a PIMPL version of a ControlDoublePrivate Shared pointer
class PollingControlProxy {
  public:
    PollingControlProxy(ControlFlags flags = ControlFlag::None)
            : PollingControlProxy(ConfigKey(), flags) {
    }

    PollingControlProxy(const QString& g, const QString& i, ControlFlags flags = ControlFlag::None)
            : PollingControlProxy(ConfigKey(g, i), flags) {
    }

    PollingControlProxy(const ConfigKey& key, ControlFlags flags = ControlFlag::None) {
        m_pControl = ControlDoublePrivate::getControl(key, flags);
        if (!m_pControl) {
            DEBUG_ASSERT(flags & ControlFlag::AllowMissingOrInvalid);
            m_pControl = ControlDoublePrivate::getDefaultControl();
        }
        DEBUG_ASSERT(m_pControl);
    }

    bool valid() const {
        return m_pControl->getKey().isValid();
    }

    /// Returns the value of the object. Thread safe, non-blocking.
    double get() const {
        return m_pControl->get();
    }

    /// Returns the bool interpretation of the value
    bool toBool() const {
        return get() > 0.0;
    }

    /// Returns the normalized value of the object. Thread safe, non-blocking.
    double getNormalizedValue() const {
        return m_pControl->getNormalizedValue();
    }

    /// Returns the normalized value of the object. Thread safe, non-blocking.
    double getNormalizedValueForValue(double value) const {
        return m_pControl->getNormalizedValueForValue(value);
    }

    /// Returns the default value of the object. Thread safe, non-blocking.
    double getDefault() const {
        return m_pControl->defaultValue();
    }

    /// Sets the control value to v. Thread safe, non-blocking.
    void set(double v) {
        m_pControl->set(v, nullptr);
    }
    /// Sets the control normalized value to v. Thread safe, non-blocking.
    void setNormalizedValue(double v) {
        m_pControl->setNormalizedValue(v, nullptr);
    }

  private:
    // not null
    QSharedPointer<ControlDoublePrivate> m_pControl;
};
