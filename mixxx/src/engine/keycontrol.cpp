// bpmcontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "engine/keycontrol.h"

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "engine/enginebuffer.h"

KeyControl::KeyControl(const char* _group,
                       ConfigObject<ConfigValue>* _config)
        : EngineControl(_group, _config) {
    m_pPitch = new ControlPotmeter(ConfigKey(_group, "pitch"), -1.f, 1.f);
    connect(m_pPitch, SIGNAL(valueChanged(double)),
            this, SLOT(slotPitchChanged(double)),
            Qt::DirectConnection);
    connect(m_pPitch, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotPitchChanged(double)),
            Qt::DirectConnection);

    m_pFileKey = new ControlObject(ConfigKey(_group, "file_key"));
    connect(m_pFileKey, SIGNAL(valueChanged(double)),
            this, SLOT(slotFileKeyChanged(double)),
            Qt::DirectConnection);

    m_pEngineKey = new ControlObject(ConfigKey(_group, "key"));
    connect(m_pEngineKey, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetEngineKey(double)),
            Qt::DirectConnection);
}

KeyControl::~KeyControl() {
    delete m_pPitch;
    delete m_pEngineKey;
    delete m_pFileKey;
}

double KeyControl::getPitchAdjust() {
    // The pitch control is measured in percentage pitch change measured in
    // octaves (-1.0 to 1.0). This conversion exponent is taken from SoundTouch.
    return exp(0.69314718056f * m_pPitch->get());
}

double KeyControl::getKey() {
    return m_pEngineKey->get();
}

void KeyControl::slotFileKeyChanged(double key) {
    // TODO(rryan): Adjust the file key by the number of octaves specified by
    // m_pPitch.
    double dRate = 1.0 + m_pPitch->get();
    m_pEngineKey->set(key * dRate);
}

void KeyControl::slotSetEngineKey(double key) {
    // TODO(rryan): set m_pPitch to match the desired key.
}

void KeyControl::slotPitchChanged(double) {
    double dFileKey = m_pFileKey->get();
    slotFileKeyChanged(dFileKey);
}
