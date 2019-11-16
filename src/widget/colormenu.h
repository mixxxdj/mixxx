#pragma once

#include <QPushButton>
#include <QMap>
#include <QWidget>

#include "util/color/color.h"

class ColorMenu : public QWidget {
    Q_OBJECT
  public:
    ColorMenu(QWidget* parent = nullptr);

    void useColorSet(PredefinedColorsRepresentation* pColorRepresentation);

  signals:
    void colorPicked(PredefinedColorPointer pColor);

  private:
    QMap<PredefinedColorPointer, QPushButton*> m_pColorButtons;
};
