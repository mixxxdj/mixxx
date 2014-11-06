/***************************************************************************
                          wstarrating.h  -  description
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

#ifndef WSTARRATING_H
#define WSTARRATING_H

#include <QEvent>

#include "library/stardelegate.h"
#include "widget/wbasewidget.h"
#include "skin/skincontext.h"

class WStarRating : public WBaseWidget {
    Q_OBJECT
  public:
    WStarRating(const char* group, ConfigObject<ConfigValue>* pConfig, QWidget* pParent);
    // WStarRating(QWidget* pParent=NULL);
    virtual ~WStarRating();

    virtual void setup(QDomNode node, const SkinContext& context);

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateRating(TrackInfoObject*);
    
  protected:
    bool event(QEvent* pEvent);
    void fillDebugTooltip(QStringList* debug);
    // Foreground and background colors.
    QColor m_qFgColor;
    QColor m_qBgColor;
    
    const char* m_pGroup;
    TrackPointer m_pCurrentTrack;
    QString m_property;
};

#endif
