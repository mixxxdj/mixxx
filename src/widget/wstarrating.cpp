/***************************************************************************
                          wstarrating.cpp  -  description
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2003 by Tue Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include <QStylePainter>
#include <QStyleOption>
#include <QSize>

#include "widget/wstarrating.h"


// #include "widget/wskincolor.h"


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
	
    // Used by delegates (e.g. StarDelegate) to tell when the mouse enters a
    // cell.
    setMouseTracking(true);
	
    /*
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);
    // painter.drawPrimitive(QStyle::PE_Widget, option);
    // painter.drawPrimitive(QStyle::PE_FrameGroupBox, option);
    // painter.drawPrimitive(QStyle::PE_PanelButtonCommand, option);
    
    // Computing the size of the widget including border,
    // margin and padding sizes
    m_widgetSize = painter.style()->sizeFromContents(
        QStyle::CT_ItemViewItem, &option, m_starRating.sizeHint());
        // QStyle::CT_LineEdit, &option, m_starRating.sizeHint());
        // QStyle::CT_PushButton, &option, m_starRating.sizeHint());
        // QStyle::CT_GroupBox, &option, m_starRating.sizeHint(), this);
    
    m_contentRect.setRect(
        (m_widgetSize.width() - m_starRating.sizeHint().width() ) / 2,
        (m_widgetSize.height() - m_starRating.sizeHint().height() )/2,
        m_starRating.sizeHint().width(),
        m_starRating.sizeHint().height()
    );
    
    qDebug() << "!!!!!!!!!!!!!!!! size " << m_widgetSize;
    qDebug() << "!!!!!!!!!!!!!!!! content " << m_contentRect;
    */
    
    // setFixedSize(m_widgetSize);
    
    
    
    m_contentRect.setRect(
        0, 0,
        m_starRating.sizeHint().width(),
        m_starRating.sizeHint().height()
    );
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
    // painter.drawPrimitive(QStyle::PE_Widget, option);
    // painter.drawPrimitive(QStyle::PE_FrameDefaultButton, option);
    // painter.drawPrimitive(QStyle::PE_PanelButtonCommand, option);
    // painter.drawPrimitive(QStyle::PE_FrameGroupBox, option);
    
    // m_starRating.paint(&painter, m_contentRect, option.palette,
    m_starRating.paint(&painter, m_contentRect, option.palette,
                       StarRating::ReadOnly,
                       option.state & QStyle::State_Selected);
    
    // qDebug() << "!!!!!!!! paintEvent rect " << rect();
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

