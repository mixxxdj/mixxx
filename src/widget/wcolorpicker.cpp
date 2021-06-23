#include "widget/wcolorpicker.h"

#include <QColorDialog>
#include <QMapIterator>
#include <QPushButton>
#include <QStyle>

#include "moc_wcolorpicker.cpp"
#include "util/color/color.h"
#include "util/parented_ptr.h"

namespace {
constexpr int kNumColumnsCandidates[] = {5, 4, 3};
} // namespace

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
        const int remainder = numItems % numColumnsCandidate;
        if (remainder == 0) {
            numColumns = numColumnsCandidate;
            break;
        }
        if (remainder > numColumnsRemainder) {
            numColumnsRemainder = numColumnsCandidate;
            numColumns = numColumnsCandidate;
        }
    }

    return numColumns;
}

WColorPicker::WColorPicker(Options options, const ColorPalette& palette, QWidget* parent)
        : QWidget(parent),
          m_options(options),
          m_palette(palette),
          m_pNoColorButton(nullptr),
          m_pCustomColorButton(nullptr) {
    QGridLayout* pLayout = new QGridLayout();
    pLayout->setMargin(0);
    pLayout->setContentsMargins(0, 0, 0, 0);

    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    setSizePolicy(QSizePolicy());

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
    QStyle* pStyle = QStyleFactory::create(QString("fusion"));
    pStyle->setParent(this);
    m_pStyle = parented_ptr<QStyle>(pStyle);

    setLayout(pLayout);
    addColorButtons();

    connect(this,
            &WColorPicker::colorPicked,
            this,
            &WColorPicker::slotColorPicked);
}

void WColorPicker::removeColorButtons() {
    QGridLayout* pLayout = qobject_cast<QGridLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(pLayout) {
        qWarning() << "Color Picker has no layout!";
        return;
    }

    if (m_pCustomColorButton) {
        pLayout->removeWidget(m_pCustomColorButton);
        delete m_pCustomColorButton;
        m_pCustomColorButton = nullptr;
    }

    while (!m_colorButtons.isEmpty()) {
        QPushButton* pColorButton = m_colorButtons.takeLast();
        pLayout->removeWidget(pColorButton);
        delete pColorButton;
    }

    if (m_pNoColorButton) {
        pLayout->removeWidget(m_pNoColorButton);
        delete m_pNoColorButton;
        m_pNoColorButton = nullptr;
    }
}

void WColorPicker::addColorButtons() {
    QGridLayout* pLayout = qobject_cast<QGridLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(pLayout) {
        qWarning() << "Color Picker has no layout!";
        return;
    }

    int row = 0;
    int column = 0;

    int numColors = m_palette.size();
    if (m_options.testFlag(Option::AllowNoColor)) {
        numColors++;
    }
    if (m_options.testFlag(Option::AllowCustomColor)) {
        numColors++;
    }

    int numColumns = idealColumnCount(numColors);
    if (m_options.testFlag(Option::AllowNoColor)) {
        addNoColorButton(pLayout, row, column);
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

    if (m_options.testFlag(Option::AllowCustomColor)) {
        addCustomColorButton(pLayout, row, column);
        column++;
    }

    adjustSize();
}

void WColorPicker::addColorButton(mixxx::RgbColor color, QGridLayout* pLayout, int row, int column) {
    parented_ptr<QPushButton> pButton = make_parented<QPushButton>("", this);
    if (m_pStyle) {
        pButton->setStyle(m_pStyle);
    }

    // Set the background color of the button. This can't be overridden in skin stylesheets.
    pButton->setStyleSheet(
            QString("QPushButton { background-color: %1; }").arg(mixxx::RgbColor::toQString(color)));
    pButton->setToolTip(mixxx::RgbColor::toQString(color));
    pButton->setCheckable(true);
    m_colorButtons.append(pButton);

    connect(pButton,
            &QPushButton::clicked,
            this,
            [this, color]() {
                emit colorPicked(mixxx::RgbColor::optional(color));
            });
    pLayout->addWidget(pButton, row, column);
}

void WColorPicker::addNoColorButton(QGridLayout* pLayout, int row, int column) {
    QPushButton* pButton = m_pNoColorButton;
    if (!pButton) {
        pButton = make_parented<QPushButton>("", this);
        if (m_pStyle) {
            pButton->setStyle(m_pStyle);
        }

        pButton->setProperty("noColor", true);
        pButton->setToolTip(tr("No color"));
        pButton->setCheckable(true);
        connect(pButton,
                &QPushButton::clicked,
                this,
                [this]() {
                    emit colorPicked(std::nullopt);
                });
        m_pNoColorButton = pButton;
    }
    pLayout->addWidget(pButton, row, column);
}

void WColorPicker::addCustomColorButton(QGridLayout* pLayout, int row, int column) {
    QPushButton* pButton = m_pCustomColorButton;
    if (!pButton) {
        pButton = make_parented<QPushButton>("", this);
        if (m_pStyle) {
            pButton->setStyle(m_pStyle);
        }

        pButton->setProperty("customColor", true);
        pButton->setToolTip(tr("Custom color"));
        pButton->setCheckable(true);
        connect(pButton,
                &QPushButton::clicked,
                this,
                [this]() {
                    QColor color = QColorDialog::getColor();
                    if (color.isValid()) {
                        emit colorPicked(mixxx::RgbColor::fromQColor(color));
                    }
                });
        m_pCustomColorButton = pButton;
    }
    pLayout->addWidget(pButton, row, column);
}

void WColorPicker::setColorButtonChecked(const mixxx::RgbColor::optional_t& color, bool checked) {
    // Unset currently selected color
    QPushButton* pButton = nullptr;
    if (color) {
        int i = m_palette.indexOf(*color);
        if (i != -1) {
            pButton = m_colorButtons.at(i);
        }
    } else if (m_options.testFlag(Option::AllowNoColor)) {
        pButton = m_pNoColorButton;
    }

    if (!pButton) {
        return;
    }

    pButton->setChecked(checked);
    // This is needed to re-apply skin styles (e.g. to show/hide a checkmark icon)
    pButton->style()->unpolish(pButton);
    pButton->style()->polish(pButton);
}

void WColorPicker::resetSelectedColor() {
    setColorButtonChecked(m_selectedColor, false);
}

void WColorPicker::setSelectedColor(const mixxx::RgbColor::optional_t& color) {
    resetSelectedColor();

    m_selectedColor = color;
    setColorButtonChecked(m_selectedColor, true);
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

void WColorPicker::slotColorPicked(const mixxx::RgbColor::optional_t& color) {
    setSelectedColor(color);
}
