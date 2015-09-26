#include "controllinpotmeter.h"

ControlLinPotmeter::ControlLinPotmeter(ConfigKey key, LinPotmeterParameters parameters)
        : ControlPotmeter(key, parameters) {
    if (m_pControl) {
        m_pControl->setBehavior(
            new ControlLinPotmeterBehavior(parameters.minValue(),
                parameters.maxValue(), parameters.neutralValue(),
                parameters.allowOutOfBounds()));
    }
    if (parameters.step()) {
        setStepCount((parameters.maxValue() - parameters.minValue())
                        / parameters.step());
    }
    if (parameters.smallStep()) {
        setSmallStepCount((parameters.maxValue() - parameters.minValue())
                        / parameters.smallStep());
    }
}



