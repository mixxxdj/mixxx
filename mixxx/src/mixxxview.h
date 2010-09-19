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
#include <Q3ValueList>
#include <QList>
#include <QTimer>

#include "trackinfoobject.h"
#include "configobject.h"

class ControlObject;
class WSlider;
class WSliderComposed;
class WPushButton;
class WDisplay;
class WKnob;
class WVisual;
class WOverview;
class WNumberPos;
class WNumberBpm;
class QDomNode;
class QDomElement;
class MixxxKeyboard;
class QComboBox;
class QLineEdit;
class QPushButton;
class QGridLayout;
class QTabWidget;
class QSplitter;
class QStackedWidget;
class WTrackTableView;
class WLibrarySidebar;
class WSearchLineEdit;
class LADSPAView;
class WaveformRenderer;
class Player;
class PlayerManager;
class QStandardItemModel;
class Library;
class WLibrary;
class RhythmboxTrackModel;
class RhythmboxPlaylistModel;
class ImgSource;

/**
 * This class provides an incomplete base for your application view.
 */

class MixxxView : public QWidget
{
    Q_OBJECT
public:
    MixxxView(QWidget *parent, MixxxKeyboard* pKeyboard,
              QString qSkinPath, ConfigObject<ConfigValue> *pConfig,
              PlayerManager* pPlayerManager,
              Library* pLibrary);
    ~MixxxView();

    /** Check if direct rendering is not available, and display warning */
    void checkDirectRendering();
    /** Return true if WVisualWaveform has been instantiated. */
    bool activeWaveform();
    static QList<QString> getSchemeList(QString qSkinPath);
private:
    static QDomElement openSkin(QString qSkinPath);

    // True if m_pVisualChX is instantiated as WVisualWaveform
    bool m_bVisualWaveform;
    MixxxKeyboard *m_pKeyboard;
    ConfigObject<ConfigValue> *m_pConfig;
    PlayerManager* m_pPlayerManager;

};

#endif
