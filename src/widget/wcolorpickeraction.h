#pragma once

#include <QHBoxLayout>
#include <QWidget>
#include <QWidgetAction>

#include "util/parented_ptr.h"
#include "widget/wcolorpicker.h"

class WColorPickerAction : public QWidgetAction {
    Q_OBJECT
  public:
    explicit WColorPickerAction(
            WColorPicker::ColorOption colorOption,
            QWidget* parent = nullptr);

    void resetSelectedColor();
    void setSelectedColor(mixxx::RgbColor::optional_t color);

  signals:
    void colorPicked(mixxx::RgbColor::optional_t color);

  private:
    parented_ptr<WColorPicker> m_pColorPicker;
};
