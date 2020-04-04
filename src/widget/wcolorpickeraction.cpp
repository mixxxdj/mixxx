#include "widget/wcolorpickeraction.h"

WColorPickerAction::WColorPickerAction(WColorPicker::Options options, const ColorPalette& palette, QWidget* parent)
        : QWidgetAction(parent),
          m_pColorPicker(make_parented<WColorPicker>(options, palette)) {
    connect(m_pColorPicker.get(), &WColorPicker::colorPicked, this, &WColorPickerAction::colorPicked);

    QHBoxLayout* pLayout = new QHBoxLayout();
    pLayout->addWidget(m_pColorPicker);

    QWidget* pWidget = new QWidget();
    pWidget->setLayout(pLayout);
    setDefaultWidget(pWidget);
}

void WColorPickerAction::resetSelectedColor() {
    m_pColorPicker->resetSelectedColor();
}

void WColorPickerAction::setSelectedColor(mixxx::RgbColor::optional_t color) {
    m_pColorPicker->setSelectedColor(color);
}

void WColorPickerAction::setColorPalette(const ColorPalette& palette) {
    m_pColorPicker->setColorPalette(palette);
}
