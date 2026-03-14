#include "engine/faderstartcontrol.h"

FaderStartControl::FaderStartControl(const QString& group) 
{

    m_pPlay=ControlObject::getControl(ConfigKey(group,"play"));
    m_pVolume=ControlObject::getControl(ConfigKey(group,"volume"));

    QObject::connect(
        m_pVolume,
        &ControlObject::valueChanged,
        [this](double value)
        {
            slotVolumeChanged(value);
        });
}

void FaderStartControl::slotVolumeChanged(double value) {

    if (value>0.01) {
        m_pPlay->set(1.0);
    } else {
        m_pPlay->set(0.0);
    }
}