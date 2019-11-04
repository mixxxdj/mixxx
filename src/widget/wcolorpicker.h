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

    void setSelectedColor(const QColor& color);
    void useColorSet(const ColorPalette& palette);

  signals:
    void colorPicked(QColor color);

  private:
    QColor m_selectedColor;
    ColorPalette m_palette;
    QList<QPushButton*> m_colorButtons;
    QStyle* m_pStyle;
};
