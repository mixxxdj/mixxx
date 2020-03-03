#include "widget/wcolorpicker.h"

#include <QMapIterator>
#include <QPushButton>
#include <QStyle>

#include "util/color/color.h"
#include "util/parented_ptr.h"

namespace {
constexpr int kNumColumnsCandidates[] = {5, 4, 3};
}

// Determine the best number of columns for items in a QGridView.
//
// Ideally, numItems % numColumn == 0 holds true. Rows that are almost
// empty do not look good, so if the we can't find the ideal column count,
// we fall back to a column count with the biggest number of elements in
// the last row.
inline int idealColumnCount(int numItems) {
    int numColumns = 4; // Default in case kNumColumnsCandidates is empty
    int numColumnsRemainder = -1;
    for (const int numColumnsCandidate : kNumColumnsCandidates) {
        int remainder = numItems % numColumnsCandidate;
        if (remainder == 0) {
            numColumns = numColumnsCandidate;
            break;
        } else if (remainder > numColumnsRemainder) {
            numColumnsRemainder = numColumnsCandidate;
            numColumns = numColumnsCandidate;
        }
    }

    return numColumns;
}

WColorPicker::WColorPicker(ColorOption colorOption, const ColorPalette& palette, QWidget* parent)
        : QWidget(parent),
          m_colorOption(colorOption),
          m_palette(palette) {
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
    setLayout(pLayout);
    addColorButtons();
}

void WColorPicker::removeColorButtons() {
    QGridLayout* pLayout = static_cast<QGridLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(pLayout) {
        qWarning() << "Color Picker has no layout!";
        return;
    }

    while (!m_colorButtons.isEmpty()) {
        QPushButton* pColorButton = m_colorButtons.takeLast();
        pLayout->removeWidget(pColorButton);
        delete pColorButton;
    }
}

void WColorPicker::addColorButtons() {
    QGridLayout* pLayout = static_cast<QGridLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(pLayout) {
        qWarning() << "Color Picker has no layout!";
        return;
    }

    int row = 0;
    int column = 0;

    int numColors = m_palette.size();
    if (m_colorOption == ColorOption::AllowNoColor) {
        numColors++;
    }

    int numColumns = idealColumnCount(numColors);
    if (m_colorOption == ColorOption::AllowNoColor) {
        addColorButton(std::nullopt, pLayout, row, column);
        column++;
    }

    for (const auto& color : m_palette) {
        addColorButton(color, pLayout, row, column);
        column++;
        if (column == numColumns) {
            column = 0;
            row++;
        }
    }
}

void WColorPicker::addColorButton(mixxx::RgbColor::optional_t color, QGridLayout* pLayout, int row, int column) {
    parented_ptr<QPushButton> pColorButton = make_parented<QPushButton>("", this);
    if (m_pStyle) {
        pColorButton->setStyle(m_pStyle);
    }

    if (color) {
        // Set the background color of the button. This can't be overridden in skin stylesheets.
        pColorButton->setStyleSheet(
                QString("QPushButton { background-color: %1; }").arg(mixxx::RgbColor::toQString(color)));
    } else {
        pColorButton->setProperty("noColor", true);
    }
    pColorButton->setToolTip(mixxx::RgbColor::toQString(color, tr("No Color")));

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
    if (m_selectedColor) {
        i = m_palette.indexOf(*m_selectedColor);
        if (i == -1) {
            return;
        }
        if (m_colorOption == ColorOption::AllowNoColor) {
            i++;
        }
    } else {
        i = 0;
        if (m_colorOption != ColorOption::AllowNoColor) {
            return;
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

void WColorPicker::setSelectedColor(mixxx::RgbColor::optional_t color) {
    resetSelectedColor();

    m_selectedColor = color;

    int i;
    if (color) {
        i = m_palette.indexOf(*color);
        if (i == -1) {
            return;
        }
        if (m_colorOption == ColorOption::AllowNoColor) {
            i++;
        }
    } else {
        i = 0;
        if (m_colorOption != ColorOption::AllowNoColor) {
            return;
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

void WColorPicker::setColorPalette(const ColorPalette& palette) {
    if (m_palette == palette) {
        return;
    }

    resetSelectedColor();
    removeColorButtons();
    m_palette = palette;
    addColorButtons();
}

void WColorPicker::slotColorPicked(mixxx::RgbColor::optional_t color) {
    setSelectedColor(color);
}
