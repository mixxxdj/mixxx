#include "widget/colormenu.h"
#include "util/color/color.h"

ColorMenu::ColorMenu(QWidget *parent)
        : QMenu(parent) {
    // If another title would be more appropriate in some context, setTitle
    // can be called again after construction.
    setTitle(tr("Set color"));
    useColorPalette(ColorPalette::mixxxHotcuesPalette);
}

void ColorMenu::useColorPalette(const ColorPalette& colorPalette) {
    clear();
    for (const auto& pColor : colorPalette.m_colorList) {
        QAction* pColorAction = new QAction(this);
        QPixmap pixmap(80, 80);
        pixmap.fill(pColor);
        pColorAction->setIcon(QIcon(pixmap));

        m_pColorActions.append(pColorAction);
        addAction(pColorAction);
        connect(pColorAction, &QAction::triggered, this, [pColor, this]() {
            emit(colorPicked(pColor));
        });
    }
}
