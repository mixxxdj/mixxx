#include "widget/wcolorpicker.h"

#include <QMapIterator>
#include <QPushButton>
#include <QStyle>

#include "util/color/color.h"
#include "util/parented_ptr.h"

namespace {
    const int kNumColumns = 4;
}

WColorPicker::WColorPicker(ColorOption colorOption, QWidget* parent)
        : QWidget(parent),
          m_colorOption(colorOption),
          m_palette(ColorPalette::mixxxPalette) {
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

    if (m_colorOption == ColorOption::AllowNoColor) {
        addColorButton(QColor(), pLayout, row, column);
        column++;
    }

    for (const auto& color : m_palette) {
        addColorButton(color, pLayout, row, column);
        column++;
        if (column == kNumColumns) {
            column = 0;
            row++;
        }
    }
}

void WColorPicker::addColorButton(const QColor& color, QGridLayout* pLayout, int row, int column) {
    setLayout(pLayout);
    parented_ptr<QPushButton> pColorButton = make_parented<QPushButton>("", this);
    if (m_pStyle) {
        pColorButton->setStyle(m_pStyle);
    }

    if (color.isValid()) {
        // Set the background color of the button. This can't be overridden in skin stylesheets.
        pColorButton->setStyleSheet(
                QString("QPushButton { background-color: %1; }").arg(color.name()));
    } else {
        pColorButton->setProperty("noColor", true);
    }

    pColorButton->setCheckable(true);
    m_colorButtons.append(pColorButton);

    pLayout->addWidget(pColorButton, row, column);

    connect(this,
            &WColorPicker::colorPicked,
            this,
            &WColorPicker::slotColorPicked);
    connect(pColorButton,
            &QPushButton::clicked,
            this,
            [color, this]() {
                emit colorPicked(color);
            });
}

void WColorPicker::resetSelectedColor() {
    // Unset currently selected color
    int i;
    if (m_colorOption == ColorOption::AllowNoColor && !m_selectedColor.isValid()) {
        i = 0;
    } else {
        i = m_palette.indexOf(m_selectedColor);
        if (i == -1) {
            return;
        }
        if (m_colorOption == ColorOption::AllowNoColor) {
            i++;
        }
    }

    DEBUG_ASSERT(i < m_colorButtons.size());

    QPushButton* pButton = m_colorButtons.at(i);
    VERIFY_OR_DEBUG_ASSERT(pButton != nullptr) {
        return;
    }
    pButton->setChecked(false);
    // This is needed to re-apply skin styles (e.g. to show/hide a checkmark icon)
    pButton->style()->unpolish(pButton);
    pButton->style()->polish(pButton);
}

void WColorPicker::setSelectedColor(const QColor& color) {
    resetSelectedColor();

    m_selectedColor = color;

    int i;
    if (m_colorOption == ColorOption::AllowNoColor && !color.isValid()) {
        i = 0;
    } else {
        i = m_palette.indexOf(color);
        if (i == -1) {
            return;
        }
        if (m_colorOption == ColorOption::AllowNoColor) {
            i++;
        }
    }

    DEBUG_ASSERT(i < m_colorButtons.size());

    QPushButton* pButton = m_colorButtons.at(i);
    VERIFY_OR_DEBUG_ASSERT(pButton != nullptr) {
        return;
    }
    pButton->setChecked(true);
    // This is needed to re-apply skin styles (e.g. to show/hide a checkmark icon)
    pButton->style()->unpolish(pButton);
    pButton->style()->polish(pButton);
}

void WColorPicker::useColorSet(const ColorPalette& palette) {
    for (int i = 0; i < m_colorButtons.size(); ++i) {
        int j = i;
        if (m_colorOption == ColorOption::AllowNoColor) {
            j++;
        }

        if (i >= palette.size() || j >= m_colorButtons.size()) {
            return;
        }

        // Set the background color of the button. This can't be overridden in skin stylesheets.
        m_colorButtons.at(j)->setStyleSheet(
                QString("QPushButton { background-color: %1; }")
                .arg(palette.at(i).name()));
    }
}


void WColorPicker::slotColorPicked(const QColor& color) {
    setSelectedColor(color);
}
