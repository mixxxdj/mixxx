#include "widget/colormenu.h"

#include "util/color/color.h"

ColorMenu::ColorMenu(QWidget* parent)
        : QMenu(parent), m_pColorDialog(new QColorDialog(this)) {
    // If another title would be more appropriate in some context, setTitle
    // can be called again after construction.
    setTitle(tr("Set color"));
    useColorPalette(ColorPalette::mixxxHotcuesPalette);

    connect(m_pColorDialog,
            &QColorDialog::colorSelected,
            this,
            [this](const QColor& newColor) {
                if (newColor.isValid()) {
                    emit(colorPicked(newColor));
                }
            });
}

void ColorMenu::useColorPalette(const ColorPalette& colorPalette) {
    clear();
    m_pActionGroup = new QActionGroup(this);
    m_pActionGroup->setExclusive(true);

    createPaletteColorsActions(colorPalette);
    createColorPickerAction();
}

void ColorMenu::setCurrentColor(QColor currentColor) {
    m_currentColor = currentColor;
    selectCurrentColorAction(currentColor);
}

void ColorMenu::openColorDialog() {
    m_pColorDialog->setCurrentColor(m_currentColor);
    m_pColorDialog->open();
}

void ColorMenu::createPaletteColorsActions(const ColorPalette& colorPalette) {
    for (const auto& color : colorPalette.m_colorList) {
        QAction* pColorAction = new QAction(m_pActionGroup);
        QPixmap pixmap(80, 80);
        pixmap.fill(color);
        pColorAction->setIcon(QIcon(pixmap));
        pColorAction->setCheckable(true);
        pColorAction->setData(color);

        addAction(pColorAction);
        connect(pColorAction, &QAction::triggered, this, [color, this]() {
            emit(colorPicked(color));
        });
    }
}

void ColorMenu::createColorPickerAction() {
    m_pColorPickerAction = new QAction(m_pActionGroup);
    m_pColorPickerAction->setText("...");
    m_pColorPickerAction->setCheckable(true);
    addAction(m_pColorPickerAction);
    connect(m_pColorPickerAction,
            &QAction::triggered,
            this,
            &ColorMenu::openColorDialog);
}

void ColorMenu::selectCurrentColorAction(const QColor& currentColor) const {
    for (QAction* pAction : actions()) {
        QColor color = pAction->data().value<QColor>();
        // Other color action is the last one, if we find it, we didn't
        // matched to any color action.
        if (color == currentColor || pAction == m_pColorPickerAction) {
            pAction->setChecked(true);
            return;
        }
    }
}
