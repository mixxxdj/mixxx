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
        : QWidget(parent),
          m_palette(HotcueColorPalette::mixxxPalette) {
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
    for (const auto& color : m_palette) {
        parented_ptr<QPushButton> pColorButton = make_parented<QPushButton>("", this);
        if (m_pStyle) {
            pColorButton->setStyle(m_pStyle);
        }

        // Set the background color of the button. This can't be overridden in skin stylesheets.
        pColorButton->setStyleSheet(
            QString("QPushButton { background-color: %1; }").arg(color.name()));

        pColorButton->setCheckable(true);
        m_colorButtons.append(pColorButton);

        pLayout->addWidget(pColorButton, row, column);
        column++;
        if (column == kNumColumns) {
            column = 0;
            row++;
        }

        connect(pColorButton, &QPushButton::clicked, this, [color, this]() {
            emit(colorPicked(color));
        });
    }
    setLayout(pLayout);
}

void WColorPicker::setSelectedColor(const QColor& color) {
    int i = m_palette.indexOf(m_selectedColor);
    if (i != -1) {
        QPushButton* pButton = m_colorButtons.at(i);
        VERIFY_OR_DEBUG_ASSERT(pButton != nullptr) {
            return;
        }
        pButton->setChecked(false);
        // This is needed to re-apply skin styles (e.g. to show/hide a checkmark icon)
        pButton->style()->unpolish(pButton);
        pButton->style()->polish(pButton);
    }

    m_selectedColor = color;

    i = m_palette.indexOf(color);
    if (i != -1) {
        QPushButton* pButton = m_colorButtons.at(i);
        VERIFY_OR_DEBUG_ASSERT(pButton != nullptr) {
            return;
        }
        pButton->setChecked(true);
        // This is needed to re-apply skin styles (e.g. to show/hide a checkmark icon)
        pButton->style()->unpolish(pButton);
        pButton->style()->polish(pButton);
    }
}

void WColorPicker::useColorSet(const HotcueColorPalette& palette) {
    for (int i = 0; i < m_colorButtons.size(); ++i) {
        if (i == palette.size()) {
            return;
        }
        // Set the background color of the button. This can't be overridden in skin stylesheets.
        m_colorButtons.at(i)->setStyleSheet(
                QString("QPushButton { background-color: %1; }")
                .arg(palette.at(i).name()));
    }
}
