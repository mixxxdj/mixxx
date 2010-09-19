/***************************************************************************
                          mixxxview.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
    copyright            : (C) 2002 by Tue and Ken .Haste Andersen
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

#include <QtCore>
#include <QtGui>

#include "mixxxview.h"
#include "widget/wwidget.h"
#include "widget/wabstractcontrol.h"
#include "widget/wknob.h"
#include "widget/wpushbutton.h"
#include "widget/wslider.h"
#include "widget/wslidercomposed.h"
#include "widget/wdisplay.h"
#include "widget/wvumeter.h"
#include "widget/wstatuslight.h"
#include "widget/wlabel.h"
#include "widget/wnumber.h"
#include "widget/wnumberpos.h"
#include "widget/wnumberbpm.h"
#include "widget/wnumberrate.h"
#include "widget/wpixmapstore.h"
#include "widget/wsearchlineedit.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrary.h"


#include "widget/woverview.h"
#include "mixxxkeyboard.h"
#include "controlobject.h"
#include "controlobjectthreadwidget.h"
#include "waveformviewerfactory.h"
#include "waveform/waveformrenderer.h"
#include "widget/wvisualsimple.h"
#include "widget/wglwaveformviewer.h"
#include "widget/wwaveformviewer.h"
#include "trackinfoobject.h"
#include "player.h"
#include "playermanager.h"

#include "skin/imgloader.h"
#include "skin/imginvert.h"
#include "skin/imgcolor.h"
#include "widget/wskincolor.h"
#include "mixxx.h"
#ifdef __LADSPA__
#include "ladspaview.h"
#endif
#include "defs_promo.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/librarytablemodel.h"
#include "library/rhythmboxtrackmodel.h"
#include "library/rhythmboxplaylistmodel.h"
#include "library/browsetablemodel.h"

MixxxView::MixxxView(QWidget* parent, MixxxKeyboard* pKeyboard,
                     QString qSkinPath, ConfigObject<ConfigValue>* pConfig,
                     PlayerManager* pPlayerManager,
                     Library* pLibrary)
        : QWidget(parent),
          m_pConfig(pConfig) {
#ifdef __WINDOWS__
#ifndef QT3_SUPPORT
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::MemoryOptim);
#endif
#endif

#ifdef __WINDOWS__
#ifndef QT3_SUPPORT
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::NormalOptim);
#endif
#endif

}

MixxxView::~MixxxView()
{

}
