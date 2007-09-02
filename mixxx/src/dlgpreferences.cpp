    /***************************************************************************
                          dlgpreferences.cpp  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
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

#ifdef __EXPERIMENTAL_RECORDING__
#include "dlgprefrecord.h"
#endif

#ifdef __EXPERIMENTAL_BPM__
#include "dlgprefbpm.h"
#endif

#ifdef __VINYLCONTROL__
#include "dlgprefvinyl.h"
#endif

#include "dlgpreferences.h"
#include "dlgprefsound.h"
#include "dlgprefmidi.h"
#include "dlgprefplaylist.h"
#include "dlgprefcontrols.h"
#include "dlgprefmixer.h"
#include "mixxx.h"
#include "track.h"
#include "wtreeview.h"
#include <QTabWidget>

#include <QTabBar>
#include <QDialog>
#include <QtGui>
//Added by qt3to4:
#include <QEvent>

DlgPreferences::DlgPreferences(MixxxApp *mixxx, MixxxView *view,
                               SoundManager* soundman,
                               Track *, ConfigObject<ConfigValue> *_config) :  QDialog(), Ui::DlgPreferencesDlg()
{
    m_pMixxx = mixxx;
    //QDialog* foo = new QDialog();

    setupUi(this);

    setWindowTitle(tr("Preferences"));
    config = _config;

    //Heavily based on the QT4 Config Dialog Example: http://doc.trolltech.com/4.3/dialogs-configdialog.html

    /*contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(96, 84));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(128);
    contentsWidget->setSpacing(12);
    */
    
    createIcons();
    //contentsWidget->setCurrentRow(0);    
    
    // Construct widgets for use in tabs
    wsound = new DlgPrefSound(this, soundman, config);
    wmidi  = new DlgPrefMidi(this, config);
    wplaylist = new DlgPrefPlaylist(this, config);
    wcontrols = new DlgPrefControls(this, view, mixxx, config);
    wmixer = new DlgPrefMixer(this, config);
#ifdef __EXPERIMENTAL_BPM__
    wbpm = new DlgPrefBPM(this, config);
#endif
#ifdef __EXPERIMENTAL_RECORDING__
    wrecord = new DlgPrefRecord(this, config);
#endif
#ifdef __VINYLCONTROL__
    wvinylcontrol = new DlgPrefVinyl(this, soundman, config);
#endif

    //pagesWidget = new QStackedWidget;
    while (pagesWidget->count() > 0)
    {
        pagesWidget->removeWidget(pagesWidget->currentWidget());
    }
       
    pagesWidget->addWidget(wsound);
    pagesWidget->addWidget(wmidi);
    pagesWidget->addWidget(wplaylist);  
    pagesWidget->addWidget(wcontrols);  
    pagesWidget->addWidget(wmixer);  
#ifdef __EXPERIMENTAL_RECORDING__
    pagesWidget->addWidget(wrecord);
#endif    
#ifdef __EXPERIMENTAL_BPM__
    pagesWidget->addWidget(wbpm);  
#endif
#ifdef __VINYLCONTROL__
    pagesWidget->addWidget(wvinylcontrol);
#endif
    // Add tabs
    /*
    addTab(wsound,    "Sound output");
    addTab(wmidi,     "Input controllers");
    addTab(wcontrols, "GUI");
    addTab(wplaylist, "Playlists");
    addTab(wmixer,    "Mixer Profile");
#ifdef __EXPERIMENTAL_BPM__
    addTab(wbpm, "BPM");
#endif

#ifdef __EXPERIMENTAL_RECORDING__
    addTab(wrecord,   "Recording");
#endif
*/

    // Add closebutton
    //setOkButton("Close");

    // Set size
    //resize(QSize(380,520));

    // Install event handler to generate closeDlg signal
    installEventFilter(this);



    // Connections
    
    connect(this, SIGNAL(showDlg()), this,      SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), wsound,    SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), wmidi,     SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), wplaylist, SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), wcontrols, SLOT(slotUpdate()));
    connect(this, SIGNAL(showDlg()), wmixer,    SLOT(slotUpdate()));
#ifdef __EXPERIMENTAL_BPM__
    connect(this, SIGNAL(showDlg()), wbpm,    SLOT(slotUpdate()));
#endif
#ifdef __EXPERIMENTAL_RECORDING__
    connect(this, SIGNAL(showDlg()), wrecord,    SLOT(slotUpdate()));
#endif
#ifdef __VINYLCONTROL__
    connect(this, SIGNAL(showDlg()), wvinylcontrol,    SLOT(slotUpdate()));
    //connect(ComboBoxSoundApi,             SIGNAL(activated(int)),    this, SLOT(slotApplyApi()));
    connect(wsound, SIGNAL(apiUpdated()), wvinylcontrol,    SLOT(slotUpdate())); //Update the vinyl control 
#endif
    connect(buttonBox, SIGNAL(accepted()), wsound,    SLOT(slotApply()));
    //connect(buttonBox, SIGNAL(accepted()), wsound,    SLOT(slotApplyApi())); //TODO: Fix this
    connect(buttonBox, SIGNAL(accepted()), wmidi,     SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), wplaylist, SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), wcontrols, SLOT(slotApply()));
    connect(buttonBox, SIGNAL(accepted()), wmixer,    SLOT(slotApply()));  
    connect(buttonBox, SIGNAL(accepted()), this,      SLOT(slotApply()));
#ifdef __EXPERIMENTAL_BPM__
    connect(buttonBox, SIGNAL(accepted()), wbpm,    SLOT(slotApply()));
#endif
#ifdef __EXPERIMENTAL_RECORDING__
    connect(buttonBox, SIGNAL(accepted()), wrecord,    SLOT(slotApply()));
#endif
#ifdef __VINYLCONTROL__
    connect(buttonBox, SIGNAL(accepted()), wvinylcontrol,    SLOT(slotApply()));
#endif    
    /*
    connect(this,        SIGNAL(buttonbox.accepted()),             wsound,    SLOT(slotApply()));
    connect(this,        SIGNAL(buttonbox.accepted()),             wsound,    SLOT(slotApplyApi()));
    connect(this,        SIGNAL(buttonbox.accepted()),             wmidi,     SLOT(slotApply()));
    connect(this,        SIGNAL(buttonbox.accepted()),             wplaylist, SLOT(slotApply()));
    connect(this,        SIGNAL(buttonbox.accepted()),             wcontrols, SLOT(slotApply()));
    connect(this,        SIGNAL(buttonbox.accepted()),             this,      SLOT(slotApply()));
    connect(this,        SIGNAL(buttonbox.accepted()),		     wmixer,    SLOT(slotApply()));*/
    //Note: The above buttonbox.accepted() things used to just be closeDlg()) - Albert June 19, 2007

//    if (tracklist->wTree)
//        connect(wplaylist,   SIGNAL(apply(QString,QString)),         tracklist->wTree, SLOT(slotSetDirs(QString,QString)));
    
    //TODO: Update the library when you change the options
    //if (view->m_pTreeView)
    //    connect(wplaylist,   SIGNAL(apply(const QString &)), view->m_pTreeView, SLOT(slotUpdateDir(const QString &)));
}

DlgPreferences::~DlgPreferences()
{
}

void DlgPreferences::createIcons()
{
    QListWidgetItem *soundButton = new QListWidgetItem(contentsWidget);
    soundButton->setIcon(QIcon(":/images/preferences/soundhardware.svg"));
    soundButton->setText(tr("Sound Hardware"));
    soundButton->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    soundButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *midiButton = new QListWidgetItem(contentsWidget);
    midiButton->setIcon(QIcon(":/images/preferences/controllers.svg"));
    midiButton->setText(tr("Input Controllers"));
    midiButton->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    midiButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *playlistButton = new QListWidgetItem(contentsWidget);
    playlistButton->setIcon(QIcon(":/images/preferences/library.svg"));
    playlistButton->setText(tr("Library and Playlists"));
    playlistButton->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    playlistButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *controlsButton = new QListWidgetItem(contentsWidget);
    controlsButton->setIcon(QIcon(":/images/preferences/interface.svg"));
    controlsButton->setText(tr("Interface"));
    controlsButton->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    controlsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *mixerButton = new QListWidgetItem(contentsWidget);
    mixerButton->setIcon(QIcon(":/images/preferences/generic.svg"));
    mixerButton->setText(tr("Mixer"));
    mixerButton->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mixerButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

#ifdef __EXPERIMENTAL_RECORDING__
    QListWidgetItem *recordingButton = new QListWidgetItem(contentsWidget);
    recordingButton->setIcon(QIcon(":/images/preferences/recording.svg"));
    recordingButton->setText(tr("Recording"));
    recordingButton->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    recordingButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif

#ifdef __EXPERIMENTAL_BPM__
    QListWidgetItem *bpmdetectButton = new QListWidgetItem(contentsWidget);
    bpmdetectButton->setIcon(QIcon(":/images/preferences/bpmdetect.svg"));
    bpmdetectButton->setText(tr("BPM Detection"));
    bpmdetectButton->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    bpmdetectButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif

#ifdef __VINYLCONTROL__
    QListWidgetItem *vinylcontrolButton = new QListWidgetItem(contentsWidget);
    //QT screws up my nice vinyl SVG for some reason, so we'll use a PNG version
    //instead...
    vinylcontrolButton->setIcon(QIcon(":/images/preferences/vinyl.png"));
    vinylcontrolButton->setText(tr("Vinyl Control"));
    vinylcontrolButton->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    vinylcontrolButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif
    connect(contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
}

void DlgPreferences::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

bool DlgPreferences::eventFilter(QObject *o, QEvent *e)
{
    // Send a close signal if dialog is closing
    if (e->type() == QEvent::Hide)
        emit(closeDlg());

    if (e->type() == QEvent::Show)
        emit(showDlg());

    // Standard event processing
    return QWidget::eventFilter(o,e);
}

void DlgPreferences::slotUpdate()
{
//    m_pMixxx->releaseKeyboard();
}

void DlgPreferences::slotApply()
{
//    m_pMixxx->grabKeyboard();
}

