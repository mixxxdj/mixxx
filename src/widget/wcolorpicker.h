#pragma once

#include <QPushButton>
#include <QMap>
#include <QWidget>
#include <QStyleFactory>

#include "util/color/color.h"

class WColorPicker : public QWidget {
    Q_OBJECT
  public:
    WColorPicker(QWidget* parent = nullptr);

    void setSelectedColor(PredefinedColorPointer pColor = nullptr);
    void useColorSet(PredefinedColorsRepresentation* pColorRepresentation);

  signals:
    void colorPicked(PredefinedColorPointer pColor);

  private:
    QMap<PredefinedColorPointer, QPushButton*> m_pColorButtons;
    PredefinedColorPointer m_pSelectedColor;
    QStyle* m_pStyle;
};
