#pragma once

#include <QPushButton>
#include <QMap>
#include <QWidget>
#include <QStyleFactory>

#include "util/color/color.h"
#include "util/color/colorpalette.h"

class WColorPicker : public QWidget {
    Q_OBJECT
  public:
    WColorPicker(QWidget* parent = nullptr);

    void setSelectedColor(QRgb color);
    void useColorPalette(const ColorPalette& colorPalette);

  signals:
    void colorPicked(QRgb color);

  private:
    QRgb m_selectedColor;
    ColorPalette m_palette;
    QList<QPushButton*> m_colorButtons;
    QStyle* m_pStyle;
};
