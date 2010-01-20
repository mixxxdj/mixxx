#include <QDebug>
#include <QModelIndexList>
#include <QModelIndex>
#include <QItemSelectionModel>
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "librarymidicontrol.h"


LibraryMIDIControl::LibraryMIDIControl(WLibrary* wlibrary, WLibrarySidebar* pSidebar) : QObject() {
    m_pLibraryWidget = wlibrary;
    m_pSidebarWidget = pSidebar;

    // Make controls for library navigation and track loading
    m_pLoadSelectedTrackCh1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]","LoadSelectedTrack")));
    m_pLoadSelectedTrackCh2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]","LoadSelectedTrack")));
    m_pSelectNextTrack = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectNextTrack")));
    m_pSelectPrevTrack = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectPrevTrack")));
    m_pSelectNextPlaylist = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectNextPlaylist")));
    m_pSelectPrevPlaylist = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectPrevPlaylist")));
    m_pLoadSelectedIntoFirstStopped = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped")));
    m_pSelectTrackKnob = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectTrackKnob")));


    connect(m_pLoadSelectedTrackCh1, SIGNAL(valueChanged(double)), this, SLOT(slotLoadSelectedTrackCh1(double)));
    connect(m_pLoadSelectedTrackCh2, SIGNAL(valueChanged(double)), this, SLOT(slotLoadSelectedTrackCh2(double)));
    connect(m_pLoadSelectedIntoFirstStopped, SIGNAL(valueChanged(double)), this, SLOT(slotLoadSelectedIntoFirstStopped(double)));
    connect(m_pSelectNextTrack, SIGNAL(valueChanged(double)), this, SLOT(slotSelectNextTrack(double)));
    connect(m_pSelectPrevTrack, SIGNAL(valueChanged(double)), this, SLOT(slotSelectPrevTrack(double)));
    connect(m_pSelectNextPlaylist, SIGNAL(valueChanged(double)), this, SLOT(slotSelectNextPlaylist(double)));
    connect(m_pSelectPrevPlaylist, SIGNAL(valueChanged(double)), this, SLOT(slotSelectPrevPlaylist(double)));
    connect(m_pSelectTrackKnob, SIGNAL(valueChanged(double)), this, SLOT(slotSelectTrackKnob(double)));
}
        
LibraryMIDIControl::~LibraryMIDIControl()
{
   delete m_pLoadSelectedTrackCh1;
   delete m_pLoadSelectedTrackCh2;
   delete m_pSelectNextTrack;
   delete m_pSelectPrevTrack;
   delete m_pSelectNextPlaylist;
   delete m_pSelectPrevPlaylist;
   delete m_pLoadSelectedIntoFirstStopped;
   delete m_pSelectTrackKnob;
}

void LibraryMIDIControl::slotLoadSelectedTrackCh1(double v)
{
    QWidget* destWidget = m_pLibraryWidget->getWidgetForMIDIControl();
    if (!destWidget) { //Some library panes may not want MIDI control. 
                       //(eg. Crates overview, which is just text...)
        return;
    }

    if (v > 0)
    {
        QApplication::postEvent(destWidget,  
                                new QKeyEvent(QEvent::KeyPress, (int)Qt::Key_BracketLeft,
                                              Qt::NoModifier, QString(), true));
    }
}
void LibraryMIDIControl::slotLoadSelectedTrackCh2(double v)
{
    QWidget* destWidget = m_pLibraryWidget->getWidgetForMIDIControl();
    if (!destWidget) { //Some library panes may not want MIDI control. 
                       //(eg. Crates overview, which is just text...)
        return;
    }

    if (v > 0)
    {
        QApplication::postEvent(destWidget,  
                                new QKeyEvent(QEvent::KeyPress, (int)Qt::Key_BracketRight,
                                              Qt::NoModifier, QString(), true));
    }
}

void LibraryMIDIControl::slotLoadSelectedIntoFirstStopped(double v)
{
    QWidget* destWidget = m_pLibraryWidget->getWidgetForMIDIControl();
    if (!destWidget) { //Some library panes may not want MIDI control. 
                       //(eg. Crates overview, which is just text...)
        return;
    }

    if (v > 0)
    {
        QApplication::postEvent(destWidget,  
                                new QKeyEvent(QEvent::KeyPress, (int)Qt::Key_Return,
                                              Qt::NoModifier, QString(), true));
    }
}

void LibraryMIDIControl::slotSelectNextTrack(double v)
{
    QWidget* destWidget = m_pLibraryWidget->getWidgetForMIDIControl();
    if (!destWidget) { //Some library panes may not want MIDI control. 
                       //(eg. Crates overview, which is just text...)
        return;
    }

    if (v > 0) 
        QApplication::postEvent(destWidget,  
                                new QKeyEvent(QEvent::KeyPress, (int)Qt::Key_Down,
                                              Qt::NoModifier, QString(), true));
    else
        QApplication::postEvent(destWidget,
                                new QKeyEvent(QEvent::KeyRelease, (int)Qt::Key_Down,
                                              Qt::NoModifier, QString(), true));
        
}
void LibraryMIDIControl::slotSelectPrevTrack(double v)
{
    QWidget* destWidget = m_pLibraryWidget->getWidgetForMIDIControl();
    if (!destWidget) { //Some library panes may not want MIDI control. 
                       //(eg. Crates overview, which is just text...)
        return;
    }

    if (v) 
        QApplication::postEvent(destWidget,
                                new QKeyEvent(QEvent::KeyPress, (int)Qt::Key_Up,
                                              Qt::NoModifier, QString(), true));
    else
        QApplication::postEvent(destWidget,
                                new QKeyEvent(QEvent::KeyRelease, (int)Qt::Key_Up,
                                              Qt::NoModifier, QString(), true));
}

void LibraryMIDIControl::slotSelectTrackKnob(double v)
{
    int i = (int)v;
    //TODO: Make sure the logic in that while loop makes sense... it looks
    //      suspicious at first glance. - Albert Dec 8, 2009

    QWidget* destWidget = m_pLibraryWidget->getWidgetForMIDIControl();
    if (!destWidget) { //Some library panes may not want MIDI control. 
                       //(eg. Crates overview, which is just text...)
        return;
    }

    while(i != 0)
    {
        if(i > 0)
        {
            QApplication::postEvent(destWidget,
                                new QKeyEvent(QEvent::KeyPress, (int)Qt::Key_Up,
                                              Qt::NoModifier, QString(), true));
            i--;
        }
        else
        {
            QApplication::postEvent(destWidget,
                                new QKeyEvent(QEvent::KeyRelease, (int)Qt::Key_Up,
                                              Qt::NoModifier, QString(), true));
            i++;
        }
    }
}


void LibraryMIDIControl::slotSelectNextPlaylist(double v)
{
    QWidget* destWidget = m_pSidebarWidget;
    if (!destWidget) {
                     
        return;
    }

    if (v > 0) 
        QApplication::postEvent(destWidget,  
                                new QKeyEvent(QEvent::KeyPress, (int)Qt::Key_Down,
                                              Qt::NoModifier, QString(), true));
    else
        QApplication::postEvent(destWidget,
                                new QKeyEvent(QEvent::KeyRelease, (int)Qt::Key_Down,
                                              Qt::NoModifier, QString(), true));
        
}
void LibraryMIDIControl::slotSelectPrevPlaylist(double v)
{
    QWidget* destWidget = m_pSidebarWidget;
    if (!destWidget) { //Some library panes may not want MIDI control. 
                       //(eg. Crates overview, which is just text...)
        return;
    }

    if (v) 
        QApplication::postEvent(destWidget,
                                new QKeyEvent(QEvent::KeyPress, (int)Qt::Key_Up,
                                              Qt::NoModifier, QString(), true));
    else
        QApplication::postEvent(destWidget,
                                new QKeyEvent(QEvent::KeyRelease, (int)Qt::Key_Up,
                                              Qt::NoModifier, QString(), true));
}

