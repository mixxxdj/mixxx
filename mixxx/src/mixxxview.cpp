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

    m_bVisualWaveform = false;

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

void MixxxView::checkDirectRendering()
{
    // IF
    //  * A waveform viewer exists
    // AND
    //  * The waveform viewer is an OpenGL waveform viewer
    // AND
    //  * The waveform viewer does not have direct rendering enabled.
    // THEN
    //  * Warn user

    // TODO rryan -- re-integrate with 'new' GL viewer

    // TODO(XXX) integrate into new skin loader


  //   if((m_pVisualCh1 &&
	// WaveformViewerFactory::getWaveformViewerType(m_pVisualCh1) == WAVEFORM_GL &&
	// !((WGLWaveformViewer *)m_pVisualCh1)->directRendering()) ||
  //      (m_pVisualCh2 &&
	// WaveformViewerFactory::getWaveformViewerType(m_pVisualCh2) == WAVEFORM_GL &&
	// !((WGLWaveformViewer *)m_pVisualCh2)->directRendering()))
  //       {
	//     if(m_pConfig->getValueString(ConfigKey("[Direct Rendering]", "Warned")) != QString("yes"))
	// 	{
	// 	    QMessageBox::warning(0, "OpenGL Direct Rendering",
	// 				 "Direct rendering is not enabled on your machine.\n\nThis means that the waveform displays will be very\nslow and take a lot of CPU time. Either update your\nconfiguration to enable direct rendering, or disable\nthe waveform displays in the control panel by\nselecting \"Simple\" under waveform displays.\nNOTE: In case you run on NVidia hardware,\ndirect rendering may not be present, but you will\nnot experience a degradation in performance.");
	// 	    m_pConfig->set(ConfigKey("[Direct Rendering]", "Warned"), ConfigValue(QString("yes")));
	// 	}
	// }

}

bool MixxxView::activeWaveform()
{
    return m_bVisualWaveform;
}

QDomElement MixxxView::openSkin(QString qSkinPath) {

    // Path to image files
    WWidget::setPixmapPath(qSkinPath.append("/"));

    // Read XML file
    QDomDocument skin("skin");
    QFile file(WWidget::getPath("skin.xml"));
    QFileInfo fi(file);
    QByteArray qbaFilename = fi.fileName().toUtf8();

    if (!file.open(QIODevice::ReadOnly))
    {
        qFatal("Could not open skin definition file: %s", qbaFilename.constData());
    }
    if (!skin.setContent(&file))
    {
        qFatal("Error parsing skin definition file: %s", qbaFilename.constData());
    }

    file.close();
    return skin.documentElement();
}


QList<QString> MixxxView::getSchemeList(QString qSkinPath) {

    QDomElement docElem = openSkin(qSkinPath);
    QList<QString> schlist;

    QDomNode colsch = docElem.namedItem("Schemes");
    if (!colsch.isNull() && colsch.isElement()) {
        QDomNode sch = colsch.firstChild();

        while (!sch.isNull()) {
            QString thisname = WWidget::selectNodeQString(sch, "Name");
            schlist.append(thisname);
            sch = sch.nextSibling();
        }
    }

    return schlist;
}

