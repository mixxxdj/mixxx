#include "widget/wstarrating.h"

#include <QMouseEvent>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wstarrating.cpp"

class QEvent;
class QWidgets;

WStarRating::WStarRating(const QString& group, QWidget* pParent)
        : WWidget(pParent),
          m_starCount(0),
          m_visualStarRating(m_starCount, 5) {
    // Controls to change the star rating with controllers.
    // Note that 'group' maybe NULLPTR, e.g. when called from DlgTrackInfo,
    // so only create rate change COs if there's a group passed when creating deck widgets.
    if (!group.isEmpty()) {
        m_pStarsUp = std::make_unique<ControlPushButton>(ConfigKey(group, "stars_up"));
        m_pStarsDown = std::make_unique<ControlPushButton>(ConfigKey(group, "stars_down"));
        connect(m_pStarsUp.get(), &ControlObject::valueChanged, this, &WStarRating::slotStarsUp);
        connect(m_pStarsDown.get(),
                &ControlObject::valueChanged,
                this,
                &WStarRating::slotStarsDown);
    }
}

void WStarRating::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
    // NOTE Here we may read <StarBreadth> from the widget,
    // then m_visualStarRating.setStarBreadth().
    // Must be <= size().height().
    // For now: YAGNI
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
    emit ratingChanged(m_starCount);
}

void WStarRating::paintEvent(QPaintEvent * /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);

    painter.setBrush(option.palette.text());
    painter.drawPrimitive(QStyle::PE_Widget, option);

    m_visualStarRating.paint(&painter, m_contentRect);
}

void WStarRating::mouseMoveEvent(QMouseEvent *event) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const int pos = event->position().toPoint().x();
#else
    const int pos = event->x();
#endif
    int star = m_visualStarRating.starAtPosition(pos, rect());

    if (star == StarRating::kInvalidStarCount) {
        resetVisualRating();
    } else {
        updateVisualRating(star);
    }
}

void WStarRating::slotStarsUp(double v) {
    if (v > 0 && m_starCount < m_visualStarRating.maxStarCount()) {
        slotSetRating(m_starCount + 1);
    }
}

void WStarRating::slotStarsDown(double v) {
    if (v > 0 && m_starCount > StarRating::kMinStarCount) {
        slotSetRating(m_starCount - 1);
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
    emit ratingChanged(starCount);
}

void WStarRating::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);

    QString currentRating;
    currentRating.setNum(m_starCount);
    QString maximumRating = QString::number(m_visualStarRating.maxStarCount());

    *debug << QString("Rating: %1/%2").arg(currentRating, maximumRating);
}
