#ifndef WHALVEDOUBLESPINBOX_H
#define WHALVEDOUBLESPINBOX_H

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "widget/wbasewidget.h"
#include "skin/skincontext.h"
#include <QDoubleSpinBox>

class ControlProxy;

class WHalveDoubleSpinBox : public QDoubleSpinBox, public WBaseWidget {
    Q_OBJECT
  public:
    WHalveDoubleSpinBox(QWidget *parent=nullptr,
                        ControlObject* pValueControl=nullptr,
                        int decimals=5,
                        double minimum=0.03125, double maximum=512.00);

  private slots:
    void slotSpinboxValueChanged(double newValue);
    void slotControlValueChanged(double newValue);

  private:
    void stepBy(int steps) override;

    ControlProxy m_valueControl;
};

#endif
