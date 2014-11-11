#include <QStylePainter>
#include <QStyleOption>
#include <QSize>

#include "widget/wstarrating.h"

WStarRating::WStarRating(QString group,
                         ConfigObject<ConfigValue>* pConfig,
                         QWidget* pParent)
        : WBaseWidget(pParent),
          m_starRating(0,5),
          m_pGroup(group),
          m_pConfig(pConfig) {
    
}

WStarRating::~WStarRating() {
}

void WStarRating::setup(QDomNode node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
    // Used by delegates (e.g. StarDelegate) to tell when the mouse enters a
    // cell.
    setMouseTracking(true);
    
    m_contentRect.setRect(0, 0, m_starRating.sizeHint().width(),
                          m_starRating.sizeHint().height());
    setFixedSize(m_starRating.sizeHint());
    
    update();
}

QSize WStarRating::sizeHint() const
{
    return m_starRating.sizeHint();
}

bool WStarRating::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QWidget::event(pEvent);
}

void WStarRating::fillDebugTooltip(QStringList* debug) {
    WBaseWidget::fillDebugTooltip(debug);
    // *debug << QString("Text: \"%1\"").arg(text());
}

void WStarRating::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.data(), SIGNAL(changed(TrackInfoObject*)),
                this, SLOT(updateRating(TrackInfoObject*)));
        updateRating();
    }
}

void WStarRating::slotTrackUnloaded(TrackPointer track) {
    Q_UNUSED(track);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), 0, this, 0);
    }
    m_pCurrentTrack.clear();
    updateRating();
}

void WStarRating::updateRating() {
    if (m_pCurrentTrack) {
        m_starRating.setStarCount(m_pCurrentTrack->getRating());
    } else {
        m_starRating.setStarCount(0);
    }
    update();
}

void WStarRating::updateRating(TrackInfoObject*) {
    updateRating();
}

void WStarRating::paintEvent(QPaintEvent *) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);
    painter.setBrush(option.palette.text());
    
    m_starRating.paint(&painter, m_contentRect, option.palette,
                       StarRating::ReadOnly,
                       option.state & QStyle::State_Selected);
}

/*
 * In the mouse event handler, we call setStarCount() on
 * the private data member m_starRating to reflect the current cursor position,
 * and we call QWidget::update() to force a repaint.
 */
 void WStarRating::mouseMoveEvent(QMouseEvent *event)
 {
    if (!m_pCurrentTrack)
        return;
    
    m_focused = true;
    int star = starAtPosition(event->x());

    if (star != m_starRating.starCount() && star != -1) {
       m_starRating.setStarCount(star);
       update();
    }
 }

 void WStarRating::leaveEvent(QEvent *){
     m_focused = false;
     updateRating();
 }

/*
 * The method uses basic linear algebra to find out which star is under the cursor.
 */
 int WStarRating::starAtPosition(int x)
 {
     // If the mouse is very close to the left edge, set 0 stars.
     if (x < m_starRating.sizeHint().width() * 0.05) {
         return 0;
     }
     int star = (x / (m_starRating.sizeHint().width() / m_starRating.maxStarCount())) + 1;

     if (star <= 0 || star > m_starRating.maxStarCount())
         return 0;

     return star;
 }

/*
 * When the user releases a mouse button, we simply emit the editingFinished() signal.
 */
void WStarRating::mouseReleaseEvent(QMouseEvent * /* event */)
{
    m_pCurrentTrack->setRating(m_starRating.starCount());
}

