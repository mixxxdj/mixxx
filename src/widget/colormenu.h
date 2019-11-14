#pragma once

#include <QColorDialog>
#include <QMenu>

#include "util/color/colorpalette.h"

class ColorMenu : public QMenu {
    Q_OBJECT
  public:
    ColorMenu(QWidget *parent = nullptr);

    void useColorPalette(const ColorPalette& colorPalette);
    void setCurrentColor(QColor currentColor);

  signals:
    void colorPicked(QColor pColor);

  private slots:
    void openColorDialog();

  private:
    void createPaletteColorsActions(const ColorPalette& colorPalette);
    void createColorPickerAction();
    void selectCurrentColorAction(const QColor& currentColor) const;

    QColor m_currentColor;
    QAction* m_pColorPickerAction;
    QActionGroup* m_pActionGroup;
    QColorDialog* m_pColorDialog;
};
