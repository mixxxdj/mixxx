#include "widget/whalvedoublespinbox.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/math.h"

WHalveDoubleSpinBox::WHalveDoubleSpinBox(QWidget * parent,
                                         ControlObject* pValueControl,
                                         int decimals,
                                         double minimum, double maximum)
        : WBaseWidget(parent),
          m_valueControl(
            pValueControl ?
            pValueControl->getKey() : ConfigKey(), this
          ) {
    setDecimals(decimals);
    setMinimum(minimum);
    setMaximum(maximum);

    connect(this, SIGNAL(valueChanged(double)),
            this, SLOT(slotSpinboxValueChanged(double)));

    setValue(m_valueControl.get());
    m_valueControl.connectValueChanged(SLOT(slotControlValueChanged(double)));
    connect(this, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateControlValue(double)));
}

void WHalveDoubleSpinBox::stepBy(int steps) {
    double newValue = value() * pow(2, steps);
    if (newValue < minimum()) {
        newValue = minimum();
    } else if (newValue > maximum()) {
        newValue = maximum();
    }
    setValue(newValue);
}

void WHalveDoubleSpinBox::slotSpinboxValueChanged(double newValue) {
    m_valueControl.set(newValue);
}

void WHalveDoubleSpinBox::slotControlValueChanged(double newValue) {
    if (value() != newValue) {
        setValue(newValue);
    }
}
