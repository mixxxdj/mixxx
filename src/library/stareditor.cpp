/***************************************************************************
                           stareditor.cpp
                              -------------------
     copyright            : (C) 2010 Tobias Rafreider
	 copyright            : (C) 2009 Nokia Corporation

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 * StarEditor inherits QWidget and is used by StarDelegate to let the user *
 * edit a star rating in the library using the mouse.                      *
 *																		   *
 * The class has been adapted from the official "Star Delegate Example",   *
 * see http://doc.trolltech.com/4.5/itemviews-stardelegate.html            *
 ***************************************************************************/

 #include <QtGui>

 #include "stareditor.h"
 #include "starrating.h"

/*
 * We enable mouse tracking on the widget so we can follow the cursor even
 * when the user doesn't hold down any mouse button. We also turn on
 * QWidget's auto-fill background feature to obtain an opaque background.
 * (Without the call, the view's background would shine through the editor.)
 */
StarEditor::StarEditor(QWidget *parent, const QStyleOptionViewItem &option)
        : QWidget(parent)
{
    setPalette(option.palette);
    setMouseTracking(true);
    setAutoFillBackground(true);
    m_isSelected = option.state & QStyle::State_Selected;
}

QSize StarEditor::sizeHint() const
 {
    return m_starRating.sizeHint();
 }
/*
 * We simply call StarRating::paint() to draw the stars,
 * just like we did when implementing StarDelegate
 */
void StarEditor::paintEvent(QPaintEvent *)
 {
    QPainter painter(this);
    m_starRating.paint(&painter, rect(), palette(), StarRating::Editable,
                       m_isSelected);
 }
/*
 * In the mouse event handler, we call setStarCount() on
 * the private data member m_starRating to reflect the current cursor position,
 * and we call QWidget::update() to force a repaint.
 */
 void StarEditor::mouseMoveEvent(QMouseEvent *event)
 {
    int star = starAtPosition(event->x());

    if (star != m_starRating.starCount() && star != -1) {
       m_starRating.setStarCount(star);
       update();
    }
 }

 void StarEditor::leaveEvent(QEvent *){
     m_starRating.setStarCount(0);
     update();
 }
/*
 * When the user releases a mouse button, we simply emit the editingFinished() signal.
 */
 void StarEditor::mouseReleaseEvent(QMouseEvent * /* event */)
 {
    emit editingFinished();
 }
/*
 * The method uses basic linear algebra to find out which star is under the cursor.
 */
 int StarEditor::starAtPosition(int x)
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
