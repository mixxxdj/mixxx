// bpmcontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "controlobject.h"
#include "controlpushbutton.h"

#include "engine/enginebuffer.h"
#include "engine/keycontrol.h"
#include "controlpotmeter.h"

#include <QDebug>

KeyControl::KeyControl(const char* _group,
                       ConfigObject<ConfigValue>* _config) :
        EngineControl(_group, _config){
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

void KeyControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }

    if (pTrack) {
        m_pTrack = pTrack;
        connect(m_pTrack.data(), SIGNAL(keyUpdated()),
                this, SLOT(slotUpdatedTrackKey()));
    }
}

void KeyControl::trackUnloaded(TrackPointer pTrack) {
    if (m_pTrack) {
        disconnect(m_pTrack.data(), SIGNAL(keyUpdated()),
                   this, SLOT(slotUpdatedTrackKey()));
    }
    m_pTrack.clear();
}

void KeyControl::slotUpdatedTrackKey() {
}

double KeyControl::convertKey(QString dValue)
{
    double key=0;
    if(dValue == "C") key=1;
    else if(dValue == "C#")key=2;
    else if(dValue == "D")key=3;
    else if(dValue == "D#")key=4;
    else if(dValue == "E")key=5;
    else if(dValue == "F")key=6;
    else if(dValue == "F#")key=7;
    else if(dValue == "G")key=8;
    else if(dValue == "G#")key=9;
    else if(dValue == "A")key=10;
    else if(dValue == "A#")key=11;
    else if(dValue == "B")key=12;
    else if(dValue == "c")key=13;
    else if(dValue == "c#")key=14;
    else if(dValue == "d")key=15;
    else if(dValue == "d#")key=16;
    else if(dValue == "e")key=17;
    else if(dValue == "f")key=18;
    else if(dValue == "f#")key=19;
    else if(dValue == "g")key=20;
    else if(dValue == "g#")key=21;
    else if(dValue == "a")key=22;
    else if(dValue == "a#")key=23;
    else if(dValue == "b")key=24;
    return key;
}

