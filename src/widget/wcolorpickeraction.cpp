#include "widget/wcolorpickeraction.h"

WColorPickerAction::WColorPickerAction(ColorPickerOption option, QWidget* parent)
        : QWidgetAction(parent) {
    m_pColorPicker = make_parented<WColorPicker>(option);
    connect(m_pColorPicker.get(), &WColorPicker::colorPicked, this, &WColorPickerAction::colorPicked);

    QHBoxLayout* pLayout = new QHBoxLayout();
    pLayout->addWidget(m_pColorPicker);

    QWidget* pWidget = new QWidget();
    pWidget->setLayout(pLayout);
    setDefaultWidget(pWidget);
}

void WColorPickerAction::setSelectedColor(PredefinedColorPointer pColor) {
    m_pColorPicker->setSelectedColor(pColor);
}
