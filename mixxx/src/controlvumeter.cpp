#include "controlvumeter.h"
#include "controlengine.h"
#include "wbulb.h"
#include "dlgvumeter.h"

// Tresholds for when the leds are to light:
const FLOAT_TYPE ControlVUmeter::m_fTresholds[6] = {0.8, 0.4, 0.2, 0.1, 0.5, 0.25};

ControlVUmeter::ControlVUmeter(ConfigKey key, DlgVUmeter *dlgVumeter) : ControlObject(key)
{
    m_fValue = 0.;

    // Set array of leds:
    m_aLeds[0] = dlgVumeter->Bulb0;
    m_aLeds[1] = dlgVumeter->Bulb1;
    m_aLeds[2] = dlgVumeter->Bulb2;
    m_aLeds[3] = dlgVumeter->Bulb3;
    m_aLeds[4] = dlgVumeter->Bulb4;
    m_aLeds[5] = dlgVumeter->Bulb5;
};

ControlVUmeter::~ControlVUmeter()
{
};

void ControlVUmeter::setValue(FLOAT_TYPE fValue)
{
    // Update the leds:
    for (int i=0; i<NLEDS; i++)
        if (fValue > m_fTresholds[i])
            m_aLeds[i]->setChecked(true);
        else
            m_aLeds[i]->setChecked(false);

    emit valueChanged(fValue);
}