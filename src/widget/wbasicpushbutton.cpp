#include "widget/wbasicpushbutton.h"

#include <QPaintEvent>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wbasicpushbutton.cpp"

namespace {
void elideButtonLabelIfNeeded(QStyleOptionButton& option,
        QStyle* style,
        QWidget* widget,
        Qt::TextElideMode elideMode) {
    if (elideMode == Qt::ElideNone) {
        return;
    }

    // Calculate how much space we have left for the text
    QSize textSize = style->subElementRect(QStyle::SE_PushButtonContents, &option, widget).size();

    if (option.features & QStyleOptionButton::HasMenu) {
        int indicatorSize = style->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, widget);
        textSize.setWidth(textSize.width() - indicatorSize);
    }

    if (!option.icon.isNull()) {
        int iconSpacing = 4; // ### 4 is currently hardcoded in QPushButton::sizeHint()
        textSize.setWidth(textSize.width() - option.iconSize.width() - iconSpacing);
    }

    // Replace overflowing text with ellipsis ("...")
    option.text = option.fontMetrics.elidedText(option.text, elideMode, textSize.width());
}
} // namespace

WBasicPushButton::WBasicPushButton(QWidget* pParent)
        : QPushButton(pParent),
          m_elideMode(Qt::ElideNone) {
}

void WBasicPushButton::setElideMode(Qt::TextElideMode elideMode) {
    if (elideMode != m_elideMode) {
        m_elideMode = elideMode;
        updateGeometry();
    }
}

QSize WBasicPushButton::minimumSizeHint() const {
    QSize fullSize = sizeHint();

    if (m_elideMode == Qt::ElideNone) {
        // Elision of overflowing text is disabled,
        // so this button cannot be collapsed beyond
        // its default size.
        return fullSize;
    } else {
        // Elision of overflowing text is enabled.
        return QSize(0, fullSize.height());
    }
}

void WBasicPushButton::paintEvent(QPaintEvent* /*unused*/) {
    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    elideButtonLabelIfNeeded(option, p.style(), this, m_elideMode);
    p.drawControl(QStyle::CE_PushButton, option);
}
