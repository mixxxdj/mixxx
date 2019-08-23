#include "widget/colormenu.h"
#include "util/color/color.h"

ColorMenu::ColorMenu(QWidget *parent, PredefinedColorsRepresentation* pColorRepresentation)
        : QMenu(parent) {
    // If another title would be more appropriate in some context, setTitle
    // can be called again after construction.
    setTitle(tr("Set color"));
}

void ColorMenu::useColorSet(PredefinedColorsRepresentation* pColorRepresentation) {
    clear();
    for (const auto& pColor : Color::kPredefinedColorsSet.allColors) {
        if (*pColor == *Color::kPredefinedColorsSet.noColor) {
            continue;
        }

        QAction* pColorAction = new QAction(pColor->m_sDisplayName, this);
        QPixmap pixmap(80, 80);
        if (pColorRepresentation == nullptr) {
            pixmap.fill(pColor->m_defaultRgba);
        } else {
            pixmap.fill(pColorRepresentation->representationFor(pColor));
        }
        pColorAction->setIcon(QIcon(pixmap));

        m_pColorActions.append(pColorAction);
        addAction(pColorAction);
        connect(pColorAction, &QAction::triggered, this, [pColor, this]() {
            emit(colorPicked(pColor));
        });
    }
}

void ColorMenu::clear() {
    for (auto& pAction : m_pColorActions) {
        if (pAction != nullptr) {
            delete pAction;
        }
    }
    m_pColorActions.clear();
}

ColorMenu::~ColorMenu() {
    clear();
}
