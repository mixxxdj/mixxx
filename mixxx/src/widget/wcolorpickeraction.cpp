#include "widget/wcolorpickeraction.h"

#include <QHBoxLayout>
#include <memory>

#include "moc_wcolorpickeraction.cpp"

WColorPickerAction::WColorPickerAction(WColorPicker::Options options, const ColorPalette& palette, QWidget* parent)
        : QWidgetAction(parent) {
    auto pWidget = std::make_unique<QWidget>();
    auto pLayout = make_parented<QHBoxLayout>(pWidget.get());
    pWidget->setLayout(pLayout);
    pWidget->setSizePolicy(QSizePolicy());
    m_pColorPicker = make_parented<WColorPicker>(options, palette, pWidget.get());
    pLayout->addWidget(m_pColorPicker);
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    setDefaultWidget(pWidget.release());
    connect(m_pColorPicker.get(),
            &WColorPicker::colorPicked,
            this,
            &WColorPickerAction::colorPicked);
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
