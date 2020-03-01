#pragma once

#include <QGridLayout>
#include <QPushButton>
#include <QMap>
#include <QWidget>
#include <QStyleFactory>

#include "util/color/color.h"
#include "util/color/colorpalette.h"

class WColorPicker : public QWidget {
    Q_OBJECT
  public:
    enum class ColorOption {
        DenyNoColor,
        AllowNoColor,
    };

    explicit WColorPicker(ColorOption colorOption, const ColorPalette& palette, QWidget* parent = nullptr);

    void resetSelectedColor();
    void setSelectedColor(mixxx::RgbColor::optional_t color);
    void useColorSet(const ColorPalette& palette);

  signals:
    void colorPicked(mixxx::RgbColor::optional_t color);

  private slots:
    void slotColorPicked(mixxx::RgbColor::optional_t color);

  private:
    void addColorButton(mixxx::RgbColor::optional_t color, QGridLayout* pLayout, int row, int column);
    ColorOption m_colorOption;
    mixxx::RgbColor::optional_t m_selectedColor;
    ColorPalette m_palette;
    QList<QPushButton*> m_colorButtons;
    QStyle* m_pStyle;
};
