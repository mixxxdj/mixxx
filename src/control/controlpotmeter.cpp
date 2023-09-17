#include "control/controlpotmeter.h"

#include "control/control.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "moc_controlpotmeter.cpp"

ControlPotmeter::ControlPotmeter(const ConfigKey& key,
        double dMinValue,
        double dMaxValue,
        bool allowOutOfBounds,
        bool bIgnoreNops,
        bool bTrack,
        bool bPersist,
        double defaultValue)
        : ControlObject(key, bIgnoreNops, bTrack, bPersist, defaultValue),
          m_controls(key) {
    setRange(dMinValue, dMaxValue, allowOutOfBounds);
    double default_value = dMinValue + 0.5 * (dMaxValue - dMinValue);
    setDefaultValue(default_value);
    if (!bPersist) {
        set(default_value);
    }
    //qDebug() << "" << this << ", min " << dMinValue << ", max " << dMaxValue << ", default " << default_value;

    if (m_pControl) {
        connect(m_pControl.data(),
                &ControlDoublePrivate::valueChanged,
                this,
                &ControlPotmeter::privateValueChanged,
                Qt::DirectConnection);
    }
    m_controls.setIsDefault(get() == default_value);
}

void ControlPotmeter::setStepCount(int count) {
    m_controls.setStepCount(count);
}

void ControlPotmeter::setSmallStepCount(int count) {
    m_controls.setSmallStepCount(count);
}

void ControlPotmeter::setRange(double dMinValue, double dMaxValue,
                               bool allowOutOfBounds) {
    m_bAllowOutOfBounds = allowOutOfBounds;

    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlPotmeterBehavior(dMinValue, dMaxValue, allowOutOfBounds));
    }
}

// slot
void ControlPotmeter::privateValueChanged(double dValue, QObject* pSender) {
    Q_UNUSED(pSender);
    m_controls.setIsDefault(dValue == defaultValue());
}

PotmeterControls::PotmeterControls(const ConfigKey& key)
        : m_control(key, this),
          m_controlUp(ConfigKey(
                  key.group, QString(key.item) + QStringLiteral("_up"))),
          m_controlDown(ConfigKey(
                  key.group, QString(key.item) + QStringLiteral("_down"))),
          m_controlUpSmall(ConfigKey(
                  key.group, QString(key.item) + QStringLiteral("_up_small"))),
          m_controlDownSmall(ConfigKey(key.group,
                  QString(key.item) + QStringLiteral("_down_small"))),
          m_controlSetDefault(ConfigKey(key.group,
                  QString(key.item) + QStringLiteral("_set_default"))),
          m_controlSetZero(ConfigKey(
                  key.group, QString(key.item) + QStringLiteral("_set_zero"))),
          m_controlSetOne(ConfigKey(
                  key.group, QString(key.item) + QStringLiteral("_set_one"))),
          m_controlSetMinusOne(ConfigKey(key.group,
                  QString(key.item) + QStringLiteral("_set_minus_one"))),
          m_controlToggle(ConfigKey(
                  key.group, QString(key.item) + QStringLiteral("_toggle"))),
          m_controlMinusToggle(ConfigKey(key.group,
                  QString(key.item) + QStringLiteral("_minus_toggle"))),
          m_stepCount(10),
          m_smallStepCount(100) {
    connect(&m_controlUp, &ControlPushButton::valueChanged, this, &PotmeterControls::incValue);
    connect(&m_controlDown, &ControlPushButton::valueChanged, this, &PotmeterControls::decValue);
    connect(&m_controlUpSmall,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::incSmallValue);
    connect(&m_controlDownSmall,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::decSmallValue);
    connect(&m_controlSetDefault,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::setToDefault);
    connect(&m_controlSetZero,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::setToZero);
    connect(&m_controlSetOne, &ControlPushButton::valueChanged, this, &PotmeterControls::setToOne);
    connect(&m_controlSetMinusOne,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::setToMinusOne);
    connect(&m_controlToggle,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::toggleValue);
    connect(&m_controlMinusToggle,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::toggleMinusValue);
}

PotmeterControls::~PotmeterControls() {
}

void PotmeterControls::addAlias(const ConfigKey& key) {
    m_controlUp.addAlias(ConfigKey(key.group, QString(key.item) + QStringLiteral("_up")));
    m_controlDown.addAlias(ConfigKey(key.group, QString(key.item) + QStringLiteral("_down")));
    m_controlUpSmall.addAlias(ConfigKey(
            key.group, QString(key.item) + QStringLiteral("_up_small")));
    m_controlDownSmall.addAlias(ConfigKey(
            key.group, QString(key.item) + QStringLiteral("_down_small")));
    m_controlSetDefault.addAlias(ConfigKey(
            key.group, QString(key.item) + QStringLiteral("_set_default")));
    m_controlSetZero.addAlias(ConfigKey(
            key.group, QString(key.item) + QStringLiteral("_set_zero")));
    m_controlSetOne.addAlias(ConfigKey(key.group, QString(key.item) + QStringLiteral("_set_one")));
    m_controlSetMinusOne.addAlias(ConfigKey(
            key.group, QString(key.item) + QStringLiteral("_set_minus_one")));
    m_controlToggle.addAlias(ConfigKey(key.group, QString(key.item) + QStringLiteral("_toggle")));
    m_controlMinusToggle.addAlias(ConfigKey(
            key.group, QString(key.item) + QStringLiteral("_minus_toggle")));
}

void PotmeterControls::incValue(double v) {
    if (v > 0) {
        double parameter = m_control.getParameter();
        parameter += 1.0 / m_stepCount;
        m_control.setParameter(parameter);
    }
}

void PotmeterControls::decValue(double v) {
    if (v > 0) {
        double parameter = m_control.getParameter();
        parameter -= 1.0 / m_stepCount;
        m_control.setParameter(parameter);
    }
}

void PotmeterControls::incSmallValue(double v) {
    if (v > 0) {
        double parameter = m_control.getParameter();
        parameter += 1.0 / m_smallStepCount;
        m_control.setParameter(parameter);
    }
}

void PotmeterControls::decSmallValue(double v) {
    if (v > 0) {
        double parameter = m_control.getParameter();
        parameter -= 1.0 / m_smallStepCount;
        m_control.setParameter(parameter);
    }
}

void PotmeterControls::setToZero(double v) {
    if (v > 0) {
        m_control.set(0.0);
    }
}

void PotmeterControls::setToOne(double v) {
    if (v > 0) {
        m_control.set(1.0);
    }
}

void PotmeterControls::setToMinusOne(double v) {
    if (v > 0) {
        m_control.set(-1.0);
    }
}

void PotmeterControls::setToDefault(double v) {
    if (v > 0) {
        m_control.reset();
    }
}

void PotmeterControls::toggleValue(double v) {
    if (v > 0) {
        double value = m_control.get();
        m_control.set(value > 0.0 ? 0.0 : 1.0);
    }
}

void PotmeterControls::toggleMinusValue(double v) {
    if (v > 0) {
        double value = m_control.get();
        m_control.set(value > 0.0 ? -1.0 : 1.0);
    }
}

void PotmeterControls::setIsDefault(bool isDefault) {
    m_controlSetDefault.forceSet(isDefault ? 1.0 : 0.0);
}
