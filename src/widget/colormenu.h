#pragma once

#include <QMenu>

#include "util/color/hotcuecolorpalette.h"

class ColorMenu : public QMenu {
    Q_OBJECT
  public:
    ColorMenu(QWidget *parent = nullptr);

    void useColorPalette(const HotcueColorPalette& colorPalette);

  signals:
    void colorPicked(QColor pColor);

  private:
    QList<QAction*> m_pColorActions;
};
