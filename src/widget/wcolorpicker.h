#pragma once

#include <QGridLayout>
#include <QMap>
#include <QPushButton>
#include <QStyle>
#include <QStyleFactory>
#include <QWidget>

#include "util/color/color.h"
#include "util/color/colorpalette.h"
#include "util/parented_ptr.h"

class WColorPicker : public QWidget {
    Q_OBJECT
  public:
    enum class Option {
        NoOptions = 0,
        AllowNoColor = 1,
        AllowCustomColor = 1 << 1,
    };
    Q_DECLARE_FLAGS(Options, Option);

    explicit WColorPicker(Options options, const ColorPalette& palette, QWidget* parent = nullptr);

    void resetSelectedColor();
    void setSelectedColor(const mixxx::RgbColor::optional_t& color);
    void setColorPalette(const ColorPalette& palette);

  signals:
    void colorPicked(const mixxx::RgbColor::optional_t& color);

  private slots:
    void slotColorPicked(const mixxx::RgbColor::optional_t& color);

  private:
    void setColorButtonChecked(const mixxx::RgbColor::optional_t& color, bool checked);
    void addColorButtons();
    void removeColorButtons();
    void addColorButton(mixxx::RgbColor color, QGridLayout* pLayout, int row, int column);
    void addNoColorButton(QGridLayout* pLayout, int row, int column);
    void addCustomColorButton(QGridLayout* pLayout, int row, int column);
    Options m_options;
    mixxx::RgbColor::optional_t m_selectedColor;
    ColorPalette m_palette;
    QPushButton* m_pNoColorButton;
    QPushButton* m_pCustomColorButton;
    QList<QPushButton*> m_colorButtons;
    parented_ptr<QStyle> m_pStyle;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WColorPicker::Options);
