/***************************************************************************
                          wtracktableitem.h  -  description
                             -------------------
    begin                : Mon May 5 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#ifndef WTRACKTABLEITEM_H
#define WTRACKTABLEITEM_H

#include <qtable.h>
#include <qcolor.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class QString;
class TrackInfoObject;

enum enumType { typeText, typeNumber, typeDuration };

class WTrackTableItem : public QTableItem
{
   // Q_OBJECT
private:
    enumType m_eType; // Wether an item is text or a number. Used for sorting.
public: 
    WTrackTableItem(TrackInfoObject *pTrackInfoObject, QTable *table, EditType et, const QString &text, enumType eType);
    ~WTrackTableItem();
    static void setRowColors(QColor r1, QColor r2);
    static void setBpmConfidenceColors(QColor c1, QColor c2);
    void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
    /** Return pointer to corresponding TrackInfoObject */
    TrackInfoObject *getTrackInfoObject();
    /** Used to update corresponding TrackInfoObject when the comment field is edited */
    void setContentFromEditor(QWidget *w);

private:
    static QColor kqRowColor1, kqRowColor2, kqBpmConfidenceColor1, kqBpmConfidenceColor2;
    QString key() const;
    int alignment() const;

    /** Pointer to track info object corresponding to this item */
    TrackInfoObject *m_pTrackInfoObject;
};

#endif
