/***************************************************************************
                          mixxxview.h  -  description
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

#ifndef MIXXXVIEW_H
#define MIXXXVIEW_H

#include <qwidget.h>
#include <qlabel.h>

class ControlObject;
class WSlider;
class WSliderComposed;
class WPushButton;
class WTrackTable;
class WDisplay;
class WKnob;
class WVisual;
class WNumber;

/**
 * This class provides an incomplete base for your application view. 
 */


class MixxxView : public QWidget
{
    Q_OBJECT
public:
    /** Construtor. Tries to open visuals if bVisuals is true. */
    MixxxView(QWidget *parent, bool bVisuals);
    ~MixxxView();
    /** Setup connections between widgets and ControlObjects */
    void assignWidgets(ControlObject *p);
    
    WTrackTable *m_pTrackTable;
    QLabel *m_pTextCh1, *m_pTextCh2;
    /** Pointer to WVisual widgets */
    WVisual *m_pVisualCh1, *m_pVisualCh2;
    WSliderComposed *m_pSliderRateCh1, *m_pSliderRateCh2;

private:
    const QString getPath(QString location);

    QWidget *main;
    WSliderComposed *m_pSliderCrossfader;
    WSliderComposed *m_pSliderVolumeCh1, *m_pSliderVolumeCh2;
    WSliderComposed *m_pSliderPlayposCh1, *m_pSliderPlayposCh2;
    WPushButton *m_pPlayCh1, *m_pPlayCh2;
    WDisplay *m_pVUmeterCh1, *m_pVUmeterCh2;
    WKnob *m_pVolume;
    WPushButton *m_pHeadCueCh1, *m_pHeadCueCh2;
    WPushButton *m_pCueCh1, *m_pCueCh2;
    WNumber *m_pBpmCh1, *m_pBpmCh2;
    WPushButton *m_pEndOfTrackModeCh1, *m_pEndOfTrackModeCh2;
    
    QString path;
};

#endif










