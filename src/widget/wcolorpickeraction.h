#pragma once

#include <QByteArrayData>
#include <QHBoxLayout>
#include <QString>
#include <QWidget>
#include <QWidgetAction>

#include "util/color/rgbcolor.h"
#include "util/parented_ptr.h"
#include "widget/wcolorpicker.h"

class ColorPalette;
class QObject;
class QWidget;

class WColorPickerAction : public QWidgetAction {
    Q_OBJECT
  public:
    explicit WColorPickerAction(
            WColorPicker::Options options,
            const ColorPalette& palette,
            QWidget* parent = nullptr);

    void resetSelectedColor();
    void setSelectedColor(mixxx::RgbColor::optional_t color);

    /// Set a new color palette for the underlying color picker.
    ///
    /// After calling this, your need to tell Qt that the menu size needs to be
    /// recalculated, e.g.:
    ///
    ///     m_pColorPickerAction->setColorPalette(palette);
    ///     QResizeEvent resizeEvent(QSize(), m_pMenu->size());
    ///     qApp->sendEvent(m_pMenu, &resizeEvent);
    void setColorPalette(const ColorPalette& palette);

  signals:
    void colorPicked(mixxx::RgbColor::optional_t color);

  private:
    parented_ptr<WColorPicker> m_pColorPicker;
};
