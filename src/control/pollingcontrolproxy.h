#pragma once

#include <QSharedPointer>
#include <QString>
#include <gsl/pointers>

#include "control/control.h"
#include "preferences/configobject.h"
#include "util/assert.h"

/// This is the light version of a control proxy without the QObject overhead.
/// This should be used when no signal connections are used.
/// It is basically a PIMPL version of a ControlDoublePrivate Shared pointer
class PollingControlProxy {
  public:
    struct Parameters : public ControlDoublePrivate::ParametersWithFlags {};

  private:
    gsl::not_null<QSharedPointer<ControlDoublePrivate>> fromParams(const Parameters& params) {
        auto control = ControlDoublePrivate::getControl(params);
        if (!control) {
            DEBUG_ASSERT(params.flags.testFlag(ControlFlag::AllowMissingOrInvalid));
            control = ControlDoublePrivate::getDefaultControl();
        }
        DEBUG_ASSERT(control);
        return gsl::not_null(control);
    };

  public:
    // convenience constructor. Prefer the `Parameter` version if you want to pass options
    PollingControlProxy(const ConfigKey& key)
            : PollingControlProxy(Parameters{{{key}}}) {
    }
    PollingControlProxy()
            : m_pControl(ControlDoublePrivate::getDefaultControl()) {
    }

    PollingControlProxy(const Parameters& params)
            : m_pControl(fromParams(params)) {
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

    /// Returns the parameterized value of the object. Thread safe, non-blocking.
    double getParameter() const {
        return m_pControl->getParameter();
    }

    /// Returns the parameterized value of the object. Thread safe, non-blocking.
    double getParameterForValue(double value) const {
        return m_pControl->getParameterForValue(value);
    }

    /// Returns the normalized parameter of the object. Thread safe, non-blocking.
    double getDefault() const {
        return m_pControl->defaultValue();
    }

    /// Return the key of the underlying control
    const ConfigKey& getKey() const {
        return m_pControl->getKey();
    }

    /// Sets the control value to v. Thread safe, non-blocking.
    void set(double v) {
        m_pControl->set(v, nullptr);
    }
    /// Sets the control parameterized value to v. Thread safe, non-blocking.
    void setParameter(double v) {
        m_pControl->setParameter(v, nullptr);
    }

  private:
    gsl::not_null<QSharedPointer<ControlDoublePrivate>> m_pControl;
};
