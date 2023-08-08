#include "widget/wstarrating.h"

#include <QApplication>
#include <QSize>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wstarrating.cpp"
#include "track/track.h"

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
}

QSize WStarRating::sizeHint() const {
    QStyleOption option;
    option.initFrom(this);
    const QSize preferredSize = m_visualStarRating.sizeHint();
    const QSize widgetSize = style()->sizeFromContents(QStyle::CT_PushButton,
            &option,
            preferredSize,
            this);

    // Center the rating inside the frame
    m_contentRect.setRect(
            (widgetSize.width() - preferredSize.width()) / 2,
            (widgetSize.height() - preferredSize.height()) / 2,
            preferredSize.width(),
            preferredSize.height());

    return widgetSize;
}

void WStarRating::slotSetRating(int starCount) {
    if (starCount == m_starCount) {
        // Unchanged
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
    int star = starAtPosition(event->position().toPoint().x());
#else
    int star = starAtPosition(event->x());
#endif

    if (star != -1) {
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

// The method uses basic linear algebra to find out which star is under the cursor.
int WStarRating::starAtPosition(int x) const {
    // If the mouse is very close to the left edge, set 0 stars.
    if (x < m_visualStarRating.sizeHint().width() * 0.05) {
        return 0;
    }
    int star = (x /
                       (m_visualStarRating.sizeHint().width() /
                               m_visualStarRating.maxStarCount())) +
            1;

    if (star <= 0 || star > m_visualStarRating.maxStarCount()) {
        return 0;
    }

    return star;
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
