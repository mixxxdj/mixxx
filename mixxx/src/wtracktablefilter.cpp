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
#include <Qt>
#include <QSortFilterProxyModel>
#include <QDirModel>
#include <QRegExp>


#include "wtracktablefilter.h"
#include "defs_audiofiles.h"

WTrackTableFilter::WTrackTableFilter(QModelIndex ind, QObject * parent)
    : QSortFilterProxyModel(parent),
    m_index(ind),
    m_regexp(MIXXX_SUPPORTED_AUDIO_FILETYPES_REGEX, Qt::CaseInsensitive)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool WTrackTableFilter::filterAcceptsRow(int sourceRow,
                                         const QModelIndex &sourceParent) const
{
    QString str;
    //we accept all parent
    if (sourceParent != m_index)
        return true;

    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

    //exclude .
    if (sourceModel()->data(index0).toString() == ".")
        return false;

    //Always include "..", regardless of the search string.
    if (sourceModel()->data(index0).toString() == "..") 
        return true;

    //include all folder
    //if (((QDirModel *)sourceModel())->isDir(index0))
    //    return true;

    //If it's a directory and it matches the search string, include it.
    if ((((QDirModel *)sourceModel())->isDir(index0)) && 
        sourceModel()->data(index0).toString().contains(filterRegExp()))
        return true;

    //exclude other file than mp3, ogg, etc..
    if (!sourceModel()->data(index0).toString().contains(m_regexp))
        return false;

    //exclude file not matching the search
    return (sourceModel()->data(index0).toString().contains(filterRegExp()));
}

void WTrackTableFilter::setIndex(QModelIndex index)
{
    m_index = index;
}
