#include "widget/colormenu.h"

#include <QGridLayout>
#include <QMapIterator>
#include <QPushButton>

#include "util/color/color.h"
#include "util/parented_ptr.h"

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

        parented_ptr<QPushButton> pColorButton = make_parented<QPushButton>("", this);
        pColorButton->setStyleSheet(QString("background-color: #%1;").arg(
            QString::number(pColor->m_defaultRgba.rgb(), 16)
        ));

        pColorButton->setToolTip(pColor->m_sDisplayName);
        m_pColorButtons.insert(pColor, pColorButton);

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

void ColorMenu::setSelectedColor(PredefinedColorPointer pColor) {
    qDebug() << "setSelectedColor";
    if (m_pSelectedColor) {
        qDebug() << "m_pSelectedColor";
        QMap<PredefinedColorPointer, QPushButton*>::const_iterator it = m_pColorButtons.find(m_pSelectedColor);
        if (it != m_pColorButtons.constEnd()) {
            qDebug() << it.value() << "setDown(false)";
            it.value()->setDown(false);
        }
    }

    if (pColor) {
        qDebug() << "m_pColor";
        QMap<PredefinedColorPointer, QPushButton*>::const_iterator it = m_pColorButtons.find(pColor);
        if (it != m_pColorButtons.constEnd()) {
            qDebug() << it.value() << "setDown(true)";
            it.value()->setDown(true);
        }
    }

    m_pSelectedColor = pColor;
}

void ColorMenu::useColorSet(PredefinedColorsRepresentation* pColorRepresentation) {
    QMapIterator<PredefinedColorPointer, QPushButton*> i(m_pColorButtons);
    while (i.hasNext()) {
        i.next();
        PredefinedColorPointer pColor = i.key();
        QPushButton* pColorButton = i.value();
        QColor color = (pColorRepresentation == nullptr) ? pColor->m_defaultRgba : pColorRepresentation->representationFor(pColor);
        pColorButton->setStyleSheet(QString("background-color: #%1;").arg(
            QString::number(color.rgb(), 16)
        ));

        pColorButton->setToolTip(pColor->m_sDisplayName);
    }
}
