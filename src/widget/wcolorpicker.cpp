#include "widget/wcolorpicker.h"

#include <QGridLayout>
#include <QMapIterator>
#include <QPushButton>

#include "util/color/color.h"
#include "util/parented_ptr.h"

namespace {
const int kNumColumns = 4;
}

WColorPicker::WColorPicker(QWidget* parent)
        : QWidget(parent) {
    // If another title would be more appropriate in some context, setTitle
    // can be called again after construction.
    QGridLayout* pLayout = new QGridLayout();
    pLayout->setMargin(0);
    pLayout->setContentsMargins(0, 0, 0, 0);

    m_pStyle = QStyleFactory::create(QString("fusion"));
    int row = 0;
    int column = 0;
    for (const auto& pColor : Color::kPredefinedColorsSet.allColors) {
        if (*pColor == *Color::kPredefinedColorsSet.noColor) {
            continue;
        }

        parented_ptr<QPushButton> pColorButton = make_parented<QPushButton>("", this);
        if (m_pStyle) {
            pColorButton->setStyle(m_pStyle);
        }
        pColorButton->setStyleSheet(
            QString("QPushButton { background-color: #%1; }").arg(pColor->m_defaultRgba.rgb(), 6, 16, QChar('0'))
        );

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

void WColorPicker::setSelectedColor(PredefinedColorPointer pColor) {
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

void WColorPicker::useColorSet(PredefinedColorsRepresentation* pColorRepresentation) {
    QMapIterator<PredefinedColorPointer, QPushButton*> i(m_pColorButtons);
    while (i.hasNext()) {
        i.next();
        PredefinedColorPointer pColor = i.key();
        QPushButton* pColorButton = i.value();
        QColor color = (pColorRepresentation == nullptr) ? pColor->m_defaultRgba : pColorRepresentation->representationFor(pColor);
        pColorButton->setStyleSheet(
            QString("QPushButton { background-color: #%1; }").arg(color.rgb(), 6, 16, QChar('0'))
        );

        pColorButton->setToolTip(pColor->m_sDisplayName);
    }
}
