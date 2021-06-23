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
        : m_pControl(new ControlProxy(key, this)),
          m_stepCount(10),
          m_smallStepCount(100) {
    // These controls are deleted when the ControlPotmeter is since
    // PotmeterControls is a member variable of the associated ControlPotmeter
    // and the push-button controls are parented to the PotmeterControls.

    ControlPushButton* controlUp = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_up"));
    controlUp->setParent(this);
    connect(controlUp,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::incValue);

    ControlPushButton* controlDown = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_down"));
    controlDown->setParent(this);
    connect(controlDown,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::decValue);

    ControlPushButton* controlUpSmall = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_up_small"));
    controlUpSmall->setParent(this);
    connect(controlUpSmall,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::incSmallValue);

    ControlPushButton* controlDownSmall = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_down_small"));
    controlDownSmall->setParent(this);
    connect(controlDownSmall,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::decSmallValue);

    m_pControlDefault = new ControlPushButton(
            ConfigKey(key.group, QString(key.item) + "_set_default"));
    m_pControlDefault->setParent(this);
    connect(m_pControlDefault,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::setToDefault);

    ControlPushButton* controlZero = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_set_zero"));
    controlZero->setParent(this);
    connect(controlZero,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::setToZero);

    ControlPushButton* controlOne = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_set_one"));
    controlOne->setParent(this);
    connect(controlOne,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::setToOne);

    ControlPushButton* controlMinusOne = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_set_minus_one"));
    controlMinusOne->setParent(this);
    connect(controlMinusOne, &ControlPushButton::valueChanged, this, &PotmeterControls::setToMinusOne);

    ControlPushButton* controlToggle = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_toggle"));
    controlToggle->setParent(this);
    connect(controlToggle,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::toggleValue);

    ControlPushButton* controlMinusToggle = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_minus_toggle"));
    controlMinusToggle->setParent(this);
    connect(controlMinusToggle,
            &ControlPushButton::valueChanged,
            this,
            &PotmeterControls::toggleMinusValue);
}

PotmeterControls::~PotmeterControls() {
}

void PotmeterControls::incValue(double v) {
    if (v > 0) {
        double parameter = m_pControl->getParameter();
        parameter += 1.0 / m_stepCount;
        m_pControl->setParameter(parameter);
    }
}

void PotmeterControls::decValue(double v) {
    if (v > 0) {
        double parameter = m_pControl->getParameter();
        parameter -= 1.0 / m_stepCount;
        m_pControl->setParameter(parameter);
    }
}

void PotmeterControls::incSmallValue(double v) {
    if (v > 0) {
        double parameter = m_pControl->getParameter();
        parameter += 1.0 / m_smallStepCount;
        m_pControl->setParameter(parameter);
    }
}

void PotmeterControls::decSmallValue(double v) {
    if (v > 0) {
        double parameter = m_pControl->getParameter();
        parameter -= 1.0 / m_smallStepCount;
        m_pControl->setParameter(parameter);
    }
}

void PotmeterControls::setToZero(double v) {
    if (v > 0) {
        m_pControl->set(0.0);
    }
}

void PotmeterControls::setToOne(double v) {
    if (v > 0) {
        m_pControl->set(1.0);
    }
}

void PotmeterControls::setToMinusOne(double v) {
    if (v > 0) {
        m_pControl->set(-1.0);
    }
}

void PotmeterControls::setToDefault(double v) {
    if (v > 0) {
        m_pControl->reset();
    }
}

void PotmeterControls::toggleValue(double v) {
    if (v > 0) {
        double value = m_pControl->get();
        m_pControl->set(value > 0.0 ? 0.0 : 1.0);
    }
}

void PotmeterControls::toggleMinusValue(double v) {
    if (v > 0) {
        double value = m_pControl->get();
        m_pControl->set(value > 0.0 ? -1.0 : 1.0);
    }
}

void PotmeterControls::setIsDefault(bool isDefault) {
    m_pControlDefault->forceSet(isDefault ? 1.0 : 0.0);
}
