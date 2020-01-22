#include "widget/wcolorpicker.h"

#include <QGridLayout>
#include <QMapIterator>
#include <QPushButton>
#include <QStyle>

#include "util/color/color.h"
#include "util/parented_ptr.h"

namespace {
    const int kNumColumns = 4;
}

WColorPicker::WColorPicker(QWidget* parent)
        : QWidget(parent) {
    QGridLayout* pLayout = new QGridLayout();
    pLayout->setMargin(0);
    pLayout->setContentsMargins(0, 0, 0, 0);

    // Unfortunately, not all styles supported by Qt support setting a
    // background color for QPushButtons (see
    // https://bugreports.qt.io/browse/QTBUG-11089). For example, when using
    // the gtk2 style all color buttons would be just grey. It's possible to
    // work around this by modifying the button border with a QSS stylesheet,
    // so that the QStyle will be overwritten, but as a sane default for skins
    // without styles for WColorPicker, we're setting the platform-independent
    // "Fusion" style here. This will make the buttons look slightly different
    // from the rest of the application (when not styled via QSS), but that's
    // better than having buttons without any colors (which would make the
    // color picker unusable).
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

        // Set the background color of the button. This can't be overridden in skin stylesheets.
        pColorButton->setStyleSheet(
            QString("QPushButton { background-color: #%1; }").arg(pColor->m_defaultRgba.rgb(), 6, 16, QChar('0'))
        );

        pColorButton->setToolTip(pColor->m_sDisplayName);
        pColorButton->setCheckable(true);
        m_pColorButtons.insert(pColor, pColorButton);

        pLayout->addWidget(pColorButton, row, column);
        column++;
        if (column == kNumColumns) {
            column = 0;
            row++;
        }

        connect(pColorButton, &QPushButton::clicked, this, [pColor, this]() {
            emit colorPicked(pColor);
        });
    }
    setLayout(pLayout);
}

void WColorPicker::setSelectedColor(PredefinedColorPointer pColor) {
    if (m_pSelectedColor) {
        auto it = m_pColorButtons.constFind(m_pSelectedColor);
        if (it != m_pColorButtons.constEnd()) {
            it.value()->setChecked(false);
            // This is needed to re-apply skin styles (e.g. to show/hide a checkmark icon)
            it.value()->style()->unpolish(it.value());
            it.value()->style()->polish(it.value());
        }
    }

    if (pColor) {
        auto it = m_pColorButtons.constFind(pColor);
        if (it != m_pColorButtons.constEnd()) {
            it.value()->setChecked(true);
            // This is needed to re-apply skin styles (e.g. to show/hide a checkmark icon)
            it.value()->style()->unpolish(it.value());
            it.value()->style()->polish(it.value());
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

        // Set the background color of the button. This can't be overridden in skin stylesheets.
        pColorButton->setStyleSheet(
            QString("QPushButton { background-color: #%1; }").arg(color.rgb(), 6, 16, QChar('0'))
        );

        pColorButton->setToolTip(pColor->m_sDisplayName);
    }
}
