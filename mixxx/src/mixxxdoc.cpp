/***************************************************************************
                          mixxxdoc.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mixxxdoc.h"

MixxxDoc::MixxxDoc()
{
  modified = false;
}

MixxxDoc::~MixxxDoc()
{
}

void MixxxDoc::newDoc()
{
}

bool MixxxDoc::save()
{
  return true;
}

bool MixxxDoc::saveAs(const QString &)
{
  return true;
}

bool MixxxDoc::load(const QString &)
{
  emit documentChanged();
  return true;
}

bool MixxxDoc::isModified() const
{
  return modified;
}
