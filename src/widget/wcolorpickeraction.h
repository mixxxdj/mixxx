#pragma once

#include <QHBoxLayout>
#include <QWidget>
#include <QWidgetAction>

#include "util/parented_ptr.h"
#include "widget/wcolorpicker.h"

class WColorPickerAction : public QWidgetAction {
    Q_OBJECT
  public:
    explicit WColorPickerAction(QWidget* parent);

    void setSelectedColor(PredefinedColorPointer pColor = nullptr);

  signals:
    void colorPicked(PredefinedColorPointer pColor);

  private:
    parented_ptr<WColorPicker> m_pColorPicker;
};
