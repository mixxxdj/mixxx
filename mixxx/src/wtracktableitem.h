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

/**
  *@author Tue & Ken Haste Andersen
  */

class QString;

enum enumType { typeText, typeNumber, typeDuration };

class WTrackTableItem : public QTableItem
{
   // Q_OBJECT
private:
    enumType m_eType; // Wether an item is text or a number. Used for sorting.
public: 
    WTrackTableItem(QTable *table, EditType et, const QString &text, enumType eType);
    ~WTrackTableItem();
    void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
    QString key() const;
};           

#endif
