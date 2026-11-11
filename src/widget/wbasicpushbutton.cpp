#include "widget/wbasicpushbutton.h"

#include <QPaintEvent>
#include <QStyleOption>
#include <QStylePainter>
#include <QToolTip>

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

QString WBasicPushButton::buildToolTip() const {
    // Show the button label as a tooltip when the label text
    // is partially hidden due to the button's size
    const QString txtToolTip = toolTip();
    if (!txtToolTip.isEmpty()) {
        return txtToolTip;
    }

    const QString txtLabel = text();
    const QSize currentSize = size();
    const QSize fullSize = sizeHint();

    if ((currentSize.height() < fullSize.height() ||
                currentSize.width() < fullSize.width()) &&
            !txtLabel.isEmpty()) {
        // The label text is not fully visible,
        // so show it as a tooltip
        return txtLabel;
    }

    return QString();
}

bool WBasicPushButton::event(QEvent* e) {
    switch (e->type()) {
    case QEvent::ToolTip: {
        const QString toolTipToShow = buildToolTip();
        if (!toolTipToShow.isEmpty()) {
            QToolTip::showText(static_cast<QHelpEvent*>(e)->globalPos(),
                    toolTipToShow,
                    this,
                    QRect(),
                    toolTipDuration());
        } else {
            e->ignore();
        }
        return true;
    }
    default: {
        return QPushButton::event(e);
    }
    }
}
