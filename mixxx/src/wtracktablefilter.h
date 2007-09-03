/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
/***************************************************************************
    copyright            : (C) 2007 by Cedric GESTES
    email                : ctaf42@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QSortFilterProxyModel>

#ifndef   	WTRACKTABLEFILTER_H_
# define   	WTRACKTABLEFILTER_H_
class QRegExp;

class WTrackTableFilter : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    WTrackTableFilter(QModelIndex ind, QObject *parent = 0);
    void setIndex(QModelIndex ind);


protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    QModelIndex m_index;
    QRegExp m_regexp;
 };

#endif 	    /* !WTRACKTABLEFILTER_H_ */
