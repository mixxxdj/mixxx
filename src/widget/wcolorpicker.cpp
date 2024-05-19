#include "widget/wcolorpicker.h"

#include <QColorDialog>
#include <QGridLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QStyle>
#include <QStyleFactory>

#include "moc_wcolorpicker.cpp"
#include "util/optional.h"
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
            numColumnsRemainder = remainder;
            numColumns = numColumnsCandidate;
        }
    }

    return numColumns;
}

WColorGridButton::WColorGridButton(const mixxx::RgbColor::optional_t& color,
        int row,
        int column,
        QWidget* parent)
        : QPushButton(parent), m_color(color), m_row(row), m_column(column) {
    if (color) {
        // Set the background color of the button.
        // This can't be overridden in skin stylesheets.
        setStyleSheet(
                QString("QPushButton { background-color: %1; }")
                        .arg(mixxx::RgbColor::toQString(color.value())));
        setToolTip(mixxx::RgbColor::toQString(color.value()));
    } else {
        setProperty("noColor", true);
        setToolTip(tr("No color"));
    }

    setCheckable(true);

    // Without this the button might shrink when setting the checkmark icon,
    // both here or via external stylesheets.
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
}

void WColorGridButton::keyPressEvent(QKeyEvent* event) {
    if (handleNavigation(event)) {
        // Already handled completely
    } else if (event->key() == Qt::Key_Return) {
        // Key_Return should act the same as Key_Space
        setDown(true);
    } else {
        QPushButton::keyPressEvent(event);
    }
}

void WColorGridButton::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return && !event->isAutoRepeat() && isDown()) {
        click();
    } else {
        QPushButton::keyReleaseEvent(event);
    }
}

bool WColorGridButton::handleNavigation(QKeyEvent* event) {
    QWidget* pParent = qobject_cast<QWidget*>(parent());
    if (!pParent) {
        return false;
    }

    QGridLayout* pGridLayout = qobject_cast<QGridLayout*>(pParent->layout());
    if (!pGridLayout) {
        return false;
    }

    const int maxRow = pGridLayout->rowCount() - 1;
    const int maxColumn = pGridLayout->columnCount() - 1;
    int newRow = m_row;
    int newColumn = m_column;

    switch (event->key()) {
    case Qt::Key_Up: {
        newRow = qBound(0, newRow - 1, maxRow);
        break;
    }
    case Qt::Key_Down: {
        newRow = qBound(0, newRow + 1, maxRow);
        break;
    }
    case Qt::Key_Left: {
        newColumn = qBound(0, newColumn - 1, maxColumn);
        break;
    }
    case Qt::Key_Right: {
        newColumn = qBound(0, newColumn + 1, maxColumn);
        break;
    }
    default: {
        return false;
    }
    }

    // Show the keyboard focus frame (if not yet visible)
    window()->setAttribute(Qt::WA_KeyboardFocusChange);

    QLayoutItem* pNewItem = pGridLayout->itemAtPosition(newRow, newColumn);
    if (!pNewItem) {
        // May happen when itemCount < (rowCount * columnCount),
        // i.e. when some cells in the grid are empty.
        return false;
    }

    QWidget* pNewFocus = pNewItem->widget();
    if (!pNewFocus) {
        return false;
    }

    pNewFocus->setFocus();
    return true;
}

WColorPicker::WColorPicker(Options options, const ColorPalette& palette, QWidget* parent)
        : QWidget(parent),
          m_options(options),
          m_palette(palette),
          m_pNoColorButton(nullptr),
          m_pCustomColorButton(nullptr) {
    QGridLayout* pLayout = new QGridLayout();
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
    auto pButton = make_parented<WColorGridButton>(color, row, column, this);
    if (m_pStyle) {
        pButton->setStyle(m_pStyle);
    }
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
    VERIFY_OR_DEBUG_ASSERT(!m_pNoColorButton) {
        return;
    }

    auto pButton = make_parented<WColorGridButton>(std::nullopt, row, column, this);
    if (m_pStyle) {
        pButton->setStyle(m_pStyle);
    }
    connect(pButton,
            &QPushButton::clicked,
            this,
            [this]() {
                emit colorPicked(std::nullopt);
            });
    m_pNoColorButton = pButton;
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
    if (m_options.testFlag(Option::NoExtStyleSheet)) {
        pButton->setIcon(QIcon(checked ? ":/images/ic_checkmark.svg" : ""));
    }
    // This is needed to re-apply skin styles (e.g. to show/hide a checkmark icon)
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
