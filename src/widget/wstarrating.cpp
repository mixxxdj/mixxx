#include "widget/wstarrating.h"

#include <QApplication>
#include <QSize>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wstarrating.cpp"
#include "track/track.h"

WStarRating::WStarRating(const QString& group, QWidget* pParent)
        : WWidget(pParent),
          m_starRating(0, 5),
          m_focused(false),
          m_currRating(0) {
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
    QSize widgetSize = style()->sizeFromContents(QStyle::CT_PushButton, &option,
                                                 m_starRating.sizeHint(), this);

    m_contentRect.setRect(
        (widgetSize.width() - m_starRating.sizeHint().width()) / 2,
        (widgetSize.height() - m_starRating.sizeHint().height()) / 2,
        m_starRating.sizeHint().width(),
        m_starRating.sizeHint().height()
    );

    return widgetSize;
}

void WStarRating::slotSetRating(int rating) {
    m_starRating.setStarCount(rating);
    m_currRating = rating;
    update();
}

void WStarRating::paintEvent(QPaintEvent * /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);

    painter.setBrush(option.palette.text());
    painter.drawPrimitive(QStyle::PE_Widget, option);

    m_starRating.paint(&painter, m_contentRect);
}

void WStarRating::mouseMoveEvent(QMouseEvent *event) {
    m_focused = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    int star = starAtPosition(event->position().toPoint().x());
#else
    int star = starAtPosition(event->x());
#endif

    if (star != m_starRating.starCount() && star != -1) {
        m_starRating.setStarCount(star);
        update();
    }
}

void WStarRating::slotStarsUp(double v) {
    if (v > 0 && m_currRating < m_starRating.maxStarCount()) {
        int star = m_currRating + 1;
        emit ratingChanged(star);
    }
}

void WStarRating::slotStarsDown(double v) {
    if (v > 0 && m_currRating > 0) {
        int star = m_currRating - 1;
        emit ratingChanged(star);
    }
}

void WStarRating::leaveEvent(QEvent* /*unused*/) {
    m_focused = false;
    // reset to applied track rating
    m_starRating.setStarCount(m_currRating);
    update();
}

// The method uses basic linear algebra to find out which star is under the cursor.
int WStarRating::starAtPosition(int x) {
    // If the mouse is very close to the left edge, set 0 stars.
    if (x < m_starRating.sizeHint().width() * 0.05) {
        return 0;
    }
    int star = (x / (m_starRating.sizeHint().width() / m_starRating.maxStarCount())) + 1;

    if (star <= 0 || star > m_starRating.maxStarCount()) {
        return 0;
    }

    return star;
}

void WStarRating::mouseReleaseEvent(QMouseEvent* /*unused*/) {
    emit ratingChanged(m_starRating.starCount());
}

void WStarRating::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);

    QString currentRating;
    currentRating.setNum(m_currRating);
    QString maximumRating = QString::number(m_starRating.maxStarCount());


    *debug << QString("Rating: %1/%2").arg(currentRating, maximumRating);
}
