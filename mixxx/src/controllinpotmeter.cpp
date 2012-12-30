#include "controllinpotmeter.h"
#include "defs.h"


// This control has a linear link between the m_dValue and the Midi Value
// limitation: m_dMaxValue represents the midi value of 128 and is never reached
ControlLinPotmeter::ControlLinPotmeter(ConfigKey key, double dMinValue, double dMaxValue) :
    ControlPotmeter(key, dMinValue, dMaxValue) {

}

double ControlLinPotmeter::getValueToWidget(double dValue) {
    double out = (dValue - m_dMinValue) / m_dValueRange;
    return math_min(out * 128, 127);
}

double ControlLinPotmeter::GetMidiValue() {
    double out = (m_dValue-m_dMinValue)/m_dValueRange;
    return math_min(out * 128, 127);
}

double ControlLinPotmeter::getValueFromWidget(double dValue) {
    double out = dValue / 128;
    return m_dMinValue + out * m_dValueRange;
}

void ControlLinPotmeter::setValueFromMidi(MidiOpCode o, double v) {
    Q_UNUSED(o);
    double out = v / 128;
    m_dValue = m_dMinValue + out * m_dValueRange;
    emit(valueChanged(m_dValue));
}


