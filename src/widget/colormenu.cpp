#include "widget/colormenu.h"
#include "util/color/color.h"

ColorMenu::ColorMenu(QWidget *parent)
        : QMenu(parent) {
    // If another title would be more appropriate in some context, setTitle
    // can be called again after construction.
    setTitle(tr("Set color"));
    for (const auto& pColor : Color::kPredefinedColorsSet.allColors) {
        if (*pColor == *Color::kPredefinedColorsSet.noColor) {
            continue;
        }

        QAction* pColorAction = new QAction(pColor->m_sDisplayName);
        QPixmap pixmap(80, 80);
        pixmap.fill(pColor->m_defaultRgba);
        pColorAction->setIcon(QIcon(pixmap));

        m_pColorActions.append(pColorAction);
        addAction(pColorAction);
        connect(pColorAction, &QAction::triggered, this, [pColor, this]() {
            emit(colorPicked(pColor));
        });
    }
}

ColorMenu::~ColorMenu() {
    for (auto& pAction : m_pColorActions) {
        delete pAction;
    }
}
