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
    void setSelectedColor(QColor color = QColor());

  signals:
    void colorPicked(QColor color);

  private:
    parented_ptr<WColorPicker> m_pColorPicker;
};
