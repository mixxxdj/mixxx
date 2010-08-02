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

#include "configobject.h"
#include "trackinfoobject.h"
#include "imgsource.h"

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
class Sampler;
class SamplerManager;
class QStandardItemModel;
class Library;
class WLibrary;
class WSampler;
class RhythmboxTrackModel;
class RhythmboxPlaylistModel;

/**
 * This class provides an incomplete base for your application view.
 */

class MixxxView : public QWidget
{
    Q_OBJECT
public:
    MixxxView(QWidget *parent, ConfigObject<ConfigValueKbd> *kbdconfig,
              QString qSkinPath, ConfigObject<ConfigValue> *pConfig,
              PlayerManager* pPlayerManager,
              SamplerManager* pSamplerManager,
              Library* pLibrary);
    ~MixxxView();

    /** Check if direct rendering is not available, and display warning */
    void checkDirectRendering();
    /** Return true if WVisualWaveform has been instantiated. */
    bool activeWaveform();
    /** Return a pointer to the track table view widget. */
    WTrackTableView* getTrackTableView();

    QLabel *m_pTextCh1, *m_pTextCh2;
    /** Pointer to WVisual widgets */
    QObject *m_pVisualCh1, *m_pVisualCh2;
    WaveformRenderer *m_pWaveformRendererCh1, *m_pWaveformRendererCh2;
    /** Pointer to absolute file position widgets */
    WNumberPos *m_pNumberPosCh1, *m_pNumberPosCh2;
    /** Pointer to BPM display widgets */
    WNumberBpm *m_pNumberBpmCh1, *m_pNumberBpmCh2;
    /** Pointer to rate slider widgets */
    WSliderComposed *m_pSliderRateCh1, *m_pSliderRateCh2;
    /** Pointer to SearchBox */
    QLabel *m_pSearchLabel;
    WSearchLineEdit *m_pLineEditSearch;
    /** Pointer to Search PushButton*/
    QPushButton *m_pPushButton;
    /** Pointer to overview displays */
    WOverview *m_pOverviewCh1, *m_pOverviewCh2;
    

    void rebootGUI(QWidget* parent, ConfigObject<ConfigValue> *pConfig, QString qSkinPath);

    static QList<QString> getSchemeList(QString qSkinPath);
  public slots:
    void slotSetupTrackConnectionsCh1(TrackPointer pTrack);
    void slotSetupTrackConnectionsCh2(TrackPointer pTrack);
    void slotUpdateTrackTextCh1(TrackPointer pTrack);
    void slotClearTrackTextCh1(TrackPointer pTrack);
    void slotUpdateTrackTextCh2(TrackPointer pTrack);
    void slotClearTrackTextCh2(TrackPointer pTrack);

private:
    void setupColorScheme(QDomElement docElem, ConfigObject<ConfigValue> *pConfig);
    void createAllWidgets(QDomElement docElem, QWidget* parent, ConfigObject<ConfigValue> *pConfig);
    void setupTabWidget(QDomNode node);
    void setupTrackSourceViewWidget(QDomNode node);

    ImgSource* parseFilters(QDomNode filt);
    static QDomElement openSkin(QString qSkinPath);
    /**Used for comboBox change*/
    int view;
    // True if m_pVisualChX is instantiated as WVisualWaveform
    bool m_bVisualWaveform;
    bool compareConfigKeys(QDomNode node, QString key);
    QList<QObject *> m_qWidgetList;
    /** Pointer to keyboard handler */
    MixxxKeyboard *m_pKeyboard;
    ConfigObject<ConfigValue> *m_pConfig;

    /** Tab widget, which contains several "pages" for different views */
    QStackedWidget* m_pTabWidget; //XXX: Temporarily turned this into a QStackedWidget instead of a QTabWidget to disable the tabs for 1.7.0 since LADSPA effects isn't finished.
    /** The widget containing the library/tracktable page */
    QWidget* m_pTabWidgetLibraryPage;
    /** The widget containing the effects/LADSPA page */
    QWidget* m_pTabWidgetEffectsPage;
    /** The layout for the library page. Allows stuff to resize automatically */
    QGridLayout* m_pLibraryPageLayout;
    /** The layout for the effects page. Allows stuff to resize automatically */
    QGridLayout* m_pEffectsPageLayout;

	// The splitter widget that contains the library panes
	QSplitter *m_pSplitter;
    // The library widget
    WLibrary* m_pLibraryWidget;
    // The library manager
    Library* m_pLibrary;
    // The library sidebar
    WLibrarySidebar* m_pLibrarySidebar;
    // Contains the actual library sidebar widget and the search box in a vertical box layout.
    QWidget* m_pLibrarySidebarPage;

    PlayerManager* m_pPlayerManager;
     
    SamplerManager* m_pSamplerManager;
    
    WSampler* m_pSampler;

#ifdef __LADSPA__
    LADSPAView* m_pLADSPAView;
#endif
};

#endif
