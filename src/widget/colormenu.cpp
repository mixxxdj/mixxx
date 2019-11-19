#include "widget/colormenu.h"

#include <QGridLayout>
#include <QMapIterator>
#include <QPushButton>

#include "util/color/color.h"

namespace {
const int kNumColumns = 4;
}

ColorMenu::ColorMenu(QWidget* parent)
        : QWidget(parent) {
    // If another title would be more appropriate in some context, setTitle
    // can be called again after construction.
    QGridLayout* pLayout = new QGridLayout();
    pLayout->setMargin(0);
    pLayout->setContentsMargins(0, 0, 0, 0);
    int row = 0;
    int column = 0;
    for (const auto& pColor : Color::kPredefinedColorsSet.allColors) {
        if (*pColor == *Color::kPredefinedColorsSet.noColor) {
            continue;
        }

        QPushButton* pColorButton = new QPushButton("", this);
        QPalette palette = pColorButton->palette();
        palette.setColor(QPalette::Button, pColor->m_defaultRgba);
        pColorButton->setPalette(palette);
        pColorButton->setToolTip(pColor->m_sDisplayName);

        pLayout->addWidget(pColorButton, row, column);
        column++;
        if (column == kNumColumns) {
            column = 0;
            row++;
        }

        connect(pColorButton, &QPushButton::clicked, this, [pColor, this]() {
            emit(colorPicked(pColor));
        });
    }
    setLayout(pLayout);
}

void ColorMenu::useColorSet(PredefinedColorsRepresentation* pColorRepresentation) {
    QMapIterator<PredefinedColorPointer, QPushButton*> i(m_pColorButtons);
    while (i.hasNext()) {
        i.next();
        QPixmap pixmap(80, 80);
        if (pColorRepresentation == nullptr) {
            pixmap.fill(i.key()->m_defaultRgba);
        } else {
            pixmap.fill(pColorRepresentation->representationFor(i.key()));
        }
        i.value()->setIcon(QIcon(pixmap));
    }
}
