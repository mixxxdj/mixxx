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


//Added by qt3to4:
#include <Q3ValueList>
#include <QList>
#include "configobject.h"
#include "imgsource.h"

class ControlObject;
class WSlider;
class WSliderComposed;
class WPushButton;
class WTrackTable;
class WDisplay;
class WKnob;
class WVisual;
class WOverview;
class WNumberPos;
class QDomNode;
class QDomElement;
class MixxxKeyboard;
/*new classes for new visual layout*/
class QComboBox;
class QLineEdit;
class QPushButton;
class QGridLayout;
class QTabWidget;
class QStackedWidget;
class WTrackTableView;
class DlgLADSPA; class LADSPAView;
class WaveformRenderer;

/**
 * This class provides an incomplete base for your application view.
 */


class MixxxView : public QWidget
{
    Q_OBJECT
public:
    MixxxView(QWidget *parent, ConfigObject<ConfigValueKbd> *kbdconfig, QString qSkinPath, ConfigObject<ConfigValue> *pConfig);
    ~MixxxView();

    /** Check if direct rendering is not available, and display warning */
    void checkDirectRendering();
    /** Return true if WVisualWaveform has been instantiated. */
    bool activeWaveform();
	//old tracktable
    //WTrackTable *m_pTrackTable;
	//NEW trackView
	WTrackTableView *m_pTrackTableView;
    QLabel *m_pTextCh1, *m_pTextCh2;
    /** Pointer to WVisual widgets */
    QObject *m_pVisualCh1, *m_pVisualCh2;
    WaveformRenderer *m_pWaveformRendererCh1, *m_pWaveformRendererCh2;
    /** Pointer to absolute file position widgets */
    WNumberPos *m_pNumberPosCh1, *m_pNumberPosCh2;
    /** Pointer to rate slider widgets */
    WSliderComposed *m_pSliderRateCh1, *m_pSliderRateCh2;
    /** Pointer to ComboBox*/
    QComboBox *m_pComboBox;
    //WComboBox *m_pComboBox;
    /** Pointer to SearchBox */
    QLabel *m_pSearchLabel;
    QLineEdit *m_pLineEditSearch;
    /** Pointer to Search PushButton*/
    QPushButton *m_pPushButton;
    /** Pointer to overview displays */
    WOverview *m_pOverviewCh1, *m_pOverviewCh2;

    void rebootGUI(QWidget* parent, ConfigObject<ConfigValue> *pConfig, QString qSkinPath);

    static QList<QString> getSchemeList(QString qSkinPath);

private:
    void setupColorScheme(QDomElement docElem, ConfigObject<ConfigValue> *pConfig);
    void createAllWidgets(QDomElement docElem, QWidget* parent, ConfigObject<ConfigValue> *pConfig);
    void setupTabWidget(QDomNode node);

    /*temp to change view*/
    WTrackTable *m_pTmpPlaylist;
    WTrackTable *m_pTmpPlaylist2;

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
    ConfigObject<ConfigValue> *m_pconfig;

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

#ifdef __LADSPA__
    DlgLADSPA* m_pDlgLADSPA;
    LADSPAView* m_pLADSPAView;
#endif
};

#endif
