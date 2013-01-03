#ifndef CONTROLLINPOTMETER_H
#define CONTROLLINPOTMETER_H

#include "controlpotmeter.h"

class ControlLinPotmeter : public ControlPotmeter
{
    Q_OBJECT
  public:

    ControlLinPotmeter(ConfigKey key, double dMinValue=0.0, double dMaxValue=1.0);

    double getValueToWidget(double dValue);
    double GetMidiValue();
    double getValueFromWidget(double dValue);

  protected:
    void setValueFromMidi(MidiOpCode o, double v);
};

#endif // CONTROLLINPOTMETER_H
