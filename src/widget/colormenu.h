#pragma once

#include <QMenu>

#include "util/color/color.h"

class ColorMenu : public QMenu {
    Q_OBJECT
  public:
    ColorMenu(QWidget *parent = nullptr);

    void useColorSet(PredefinedColorsRepresentation* pColorRepresentation);

  signals:
    void colorPicked(PredefinedColorPointer pColor);

  private:
    QMap<PredefinedColorPointer, QAction*> m_pColorActions;
};
