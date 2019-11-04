#pragma once

#include <QMenu>

#include "util/color/colorpalette.h"

class ColorMenu : public QMenu {
    Q_OBJECT
  public:
    ColorMenu(QWidget *parent = nullptr);

    void useColorPalette(const ColorPalette& colorPalette);

  signals:
    void colorPicked(QColor pColor);

  private:
    QList<QAction*> m_pColorActions;
};
