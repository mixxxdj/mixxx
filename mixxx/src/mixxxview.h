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
#include <qstring.h>

class ControlObject;
class WSlider;
class WSliderComposed;
class WPushButton;
class WTrackTable;
class WDisplay;
class WKnob;
class WVisual;
class WNumberPos;
class QDomNode;

/**
 * This class provides an incomplete base for your application view. 
 */


class MixxxView : public QWidget
{
    Q_OBJECT
public:
    /** Construtor. Tries to open visuals if bVisuals is true. */
    MixxxView(QWidget *parent, bool bVisualsWaveform, QString qSkinPath);
    ~MixxxView();

    /** Return true if WVisualWaveform has been instantiated. */
    bool activeWaveform();

    WTrackTable *m_pTrackTable;
    QLabel *m_pTextCh1, *m_pTextCh2;
    /** Pointer to WVisual widgets */
    QObject *m_pVisualCh1, *m_pVisualCh2;
    /** Pointer to absolute file position widgets */
    WNumberPos *m_pNumberPosCh1, *m_pNumberPosCh2;
    /** Pointer to rate slider widgets */
    WSliderComposed *m_pSliderRateCh1, *m_pSliderRateCh2;
    /** Allow dynamic zoom on visuals */
    bool m_bZoom;

protected:
    //void keyPressEvent(QKeyEvent *e);

private:
    // True if m_pVisualChX is instantiated as WVisualWaveform
    bool m_bVisualWaveform;
    bool compareConfigKeys(QDomNode node, QString key);
};

#endif










