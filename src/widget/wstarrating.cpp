#include "widget/wstarrating.h"

#include <QMouseEvent>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wstarrating.cpp"

class QEvent;
class QWidgets;

WStarRating::WStarRating(QWidget* pParent)
        : WWidget(pParent),
          m_upDownChangesFocus(false),
          m_focusedViaKeyboard(false),
          m_starCount(0),
          m_visualStarRating(m_starCount) {
    setAttribute(Qt::WA_Hover);
}

void WStarRating::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
}

QSize WStarRating::sizeHint() const {
    // Center rating horizontally and vertically
    m_contentRect.setRect(
            (size().width() - m_visualStarRating.sizeHint().width()) / 2,
            (size().height() - m_visualStarRating.sizeHint().height()) / 2,
            m_visualStarRating.sizeHint().width(),
            m_visualStarRating.sizeHint().height());

    return size();
}

void WStarRating::slotSetRating(int starCount) {
    if (starCount == m_starCount || !m_visualStarRating.verifyStarCount(starCount)) {
        return;
    }
    m_starCount = starCount;
    updateVisualRating(starCount);
}

void WStarRating::focusInEvent(QFocusEvent* event) {
    m_focusedViaKeyboard = event->reason() == Qt::TabFocusReason ||
            event->reason() == Qt::BacktabFocusReason ||
            event->reason() == Qt::ShortcutFocusReason;
    QWidget::focusInEvent(event);
}

void WStarRating::focusOutEvent(QFocusEvent* event) {
    auto shouldUpdateVisual = m_focusedViaKeyboard;
    m_focusedViaKeyboard = false;
    QWidget::focusInEvent(event);
    if (shouldUpdateVisual) {
        update();
    }
}

void WStarRating::paintEvent(QPaintEvent * /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);

    // The default value for State_KeyboardFocusChange is never
    // reset once set to true for a certain window, so we
    // implement our own custom version of it instead.
    if (m_focusedViaKeyboard) {
        option.state |= QStyle::State_KeyboardFocusChange;
    } else {
        option.state &= ~QStyle::State_KeyboardFocusChange;
    }

    // Center rating horizontally and vertically
    QRect contentRect(QPoint(0, 0), m_visualStarRating.sizeHint());
    contentRect.moveCenter(option.rect.center());

    painter.drawPrimitive(QStyle::PE_Widget, option);

    if (focusPolicy() != Qt::NoFocus &&
            (option.state & QStyle::State_HasFocus) &&
            (option.state & QStyle::State_KeyboardFocusChange)) {
        painter.setBrush(option.palette.highlight().color());
    } else {
        painter.setBrush(option.palette.text().color());
    }

    m_visualStarRating.paint(&painter, contentRect);
}

void WStarRating::keyPressEvent(QKeyEvent* event) {
    // Change rating when certain keys are pressed
    QKeyEvent* ke = static_cast<QKeyEvent*>(event);
    int newRating = m_visualStarRating.starCount();
    switch (ke->key()) {
    case Qt::Key_0: {
        newRating = 0;
        break;
    }
    case Qt::Key_1: {
        newRating = 1;
        break;
    }
    case Qt::Key_2: {
        newRating = 2;
        break;
    }
    case Qt::Key_3: {
        newRating = 3;
        break;
    }
    case Qt::Key_4: {
        newRating = 4;
        break;
    }
    case Qt::Key_5: {
        newRating = 5;
        break;
    }
    case Qt::Key_6: {
        newRating = 6;
        break;
    }
    case Qt::Key_7: {
        newRating = 7;
        break;
    }
    case Qt::Key_8: {
        newRating = 8;
        break;
    }
    case Qt::Key_9: {
        newRating = 9;
        break;
    }
    case Qt::Key_Right:
    case Qt::Key_Plus: {
        newRating++;
        break;
    }
    case Qt::Key_Left:
    case Qt::Key_Minus: {
        newRating--;
        break;
    }
    case Qt::Key_Up: {
        if (m_upDownChangesFocus && focusPreviousChild()) {
            event->accept();
        } else {
            event->ignore();
        }
        return;
    }
    case Qt::Key_Down: {
        if (m_upDownChangesFocus && focusNextChild()) {
            event->accept();
        } else {
            event->ignore();
        }
        return;
    }
    default: {
        event->ignore();
        return;
    }
    }
    bool shouldUpdateVisual = !m_focusedViaKeyboard;
    m_focusedViaKeyboard = true;
    newRating = math_clamp(newRating, StarRating::kMinStarCount, m_visualStarRating.maxStarCount());
    updateVisualRating(newRating, shouldUpdateVisual);
    m_starCount = newRating;
}

void WStarRating::mouseMoveEvent(QMouseEvent *event) {
    const int pos = event->position().toPoint().x();
    int star = m_visualStarRating.starAtPosition(pos, rect());

    if (star == StarRating::kInvalidStarCount) {
        resetVisualRating();
    } else {
        updateVisualRating(star);
    }
}

void WStarRating::leaveEvent(QEvent* /*unused*/) {
    resetVisualRating();
}

void WStarRating::updateVisualRating(int starCount, bool forceRepaint) {
    if (starCount == m_visualStarRating.starCount()) {
        if (forceRepaint) {
            update();
        }
        return;
    }
    m_visualStarRating.setStarCount(starCount);
    update();
}

void WStarRating::mouseReleaseEvent(QMouseEvent* /*unused*/) {
    int starCount = m_visualStarRating.starCount();
    emit ratingChangeRequest(starCount);
}

void WStarRating::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);

    QString currentRating;
    currentRating.setNum(m_starCount);
    QString maximumRating = QString::number(m_visualStarRating.maxStarCount());

    *debug << QString("Rating: %1/%2").arg(currentRating, maximumRating);
}
