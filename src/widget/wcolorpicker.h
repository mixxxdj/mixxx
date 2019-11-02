#pragma once

#include <QGridLayout>
#include <QPushButton>
#include <QMap>
#include <QWidget>
#include <QStyleFactory>

#include "util/color/color.h"
#include "util/color/hotcuecolorpalette.h"

class WColorPicker : public QWidget {
    Q_OBJECT
  public:
    enum class ColorOption {
        DenyNoColor,
        AllowNoColor,
    };

    explicit WColorPicker(ColorOption colorOption, QWidget* parent = nullptr);

    void resetSelectedColor();
    void setSelectedColor(const QColor& color);
    void useColorSet(const HotcueColorPalette& palette);

  signals:
    void colorPicked(QColor color);

  private slots:
    void slotColorPicked(const QColor& color);

  private:
    void addColorButton(const QColor& color, QGridLayout* pLayout, int row, int column);
    ColorOption m_colorOption;
    QColor m_selectedColor;
    HotcueColorPalette m_palette;
    QList<QPushButton*> m_colorButtons;
    QStyle* m_pStyle;
};
