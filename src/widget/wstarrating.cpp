#include "widget/wstarrating.h"

#include <QMouseEvent>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wstarrating.cpp"

class QEvent;
class QWidgets;

WStarRating::WStarRating(QWidget* pParent)
        : WWidget(pParent),
          m_starCount(0),
          m_visualStarRating(m_starCount) {
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

void WStarRating::paintEvent(QPaintEvent * /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);

    painter.setBrush(option.palette.text());
    painter.drawPrimitive(QStyle::PE_Widget, option);

    m_visualStarRating.paint(&painter, m_contentRect);
}

void WStarRating::keyPressEvent(QKeyEvent* event) {
    // Change rating when certain keys are pressed
    QKeyEvent* ke = static_cast<QKeyEvent*>(event);
    int newRating = m_visualStarRating.starCount();
    switch (ke->key()) {
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9: {
        bool ok = false;
        int keyInt = ke->text().toInt(&ok);
        if (ok) {
            newRating = keyInt;
        }
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
    default: {
        event->ignore();
        return;
    }
    }
    newRating = math_clamp(newRating, StarRating::kMinStarCount, m_visualStarRating.maxStarCount());
    updateVisualRating(newRating);
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

void WStarRating::updateVisualRating(int starCount) {
    if (starCount == m_visualStarRating.starCount()) {
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
