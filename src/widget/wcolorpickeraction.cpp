#include "widget/wcolorpickeraction.h"

#include "moc_wcolorpickeraction.cpp"

WColorPickerAction::WColorPickerAction(WColorPicker::Options options, const ColorPalette& palette, QWidget* parent)
        : QWidgetAction(parent),
          m_pColorPicker(make_parented<WColorPicker>(options, palette)) {
    connect(m_pColorPicker.get(), &WColorPicker::colorPicked, this, &WColorPickerAction::colorPicked);

    QHBoxLayout* pLayout = new QHBoxLayout();
    pLayout->addWidget(m_pColorPicker);
    pLayout->setSizeConstraint(QLayout::SetFixedSize);

    QWidget* pWidget = new QWidget();
    pWidget->setLayout(pLayout);
    pWidget->setSizePolicy(QSizePolicy());
    setDefaultWidget(pWidget);
}

void WColorPickerAction::resetSelectedColor() {
    m_pColorPicker->resetSelectedColor();
}

void WColorPickerAction::setSelectedColor(const mixxx::RgbColor::optional_t& color) {
    m_pColorPicker->setSelectedColor(color);
}

void WColorPickerAction::setColorPalette(const ColorPalette& palette) {
    m_pColorPicker->setColorPalette(palette);
    QWidget* pWidget = defaultWidget();
    VERIFY_OR_DEBUG_ASSERT(pWidget) {
        return;
    }
    pWidget->adjustSize();
}
