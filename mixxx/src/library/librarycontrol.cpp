#include <QDebug>
#include <QModelIndexList>
#include <QModelIndex>
#include <QItemSelectionModel>

#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "library/librarycontrol.h"
#include "library/libraryview.h"

LibraryControl::LibraryControl(QObject* pParent) : QObject(pParent) {
    m_pLibraryWidget = NULL;
    m_pSidebarWidget = NULL;

    // Make controls for library navigation and track loading. Leaking all these CO's, but oh well?

    m_pLoadSelectedTrackCh1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]","LoadSelectedTrack")));
    connect(m_pLoadSelectedTrackCh1, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadSelectedTrackCh1(double)));

    m_pLoadSelectedTrackCh2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]","LoadSelectedTrack")));
    connect(m_pLoadSelectedTrackCh2, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadSelectedTrackCh2(double)));

    m_pSelectNextTrack = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectNextTrack")));
    connect(m_pSelectNextTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextTrack(double)));

    m_pSelectPrevTrack = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectPrevTrack")));
    connect(m_pSelectPrevTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevTrack(double)));

    m_pSelectNextPlaylist = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectNextPlaylist")));
    connect(m_pSelectNextPlaylist, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextPlaylist(double)));

    m_pSelectPrevPlaylist = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectPrevPlaylist")));
    connect(m_pSelectPrevPlaylist, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevPlaylist(double)));

    m_pLoadSelectedIntoFirstStopped = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped")));
    connect(m_pLoadSelectedIntoFirstStopped, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadSelectedIntoFirstStopped(double)));

    m_pSelectTrackKnob = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectTrackKnob")));
    connect(m_pSelectTrackKnob, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectTrackKnob(double)));
}

LibraryControl::~LibraryControl() {
   delete m_pLoadSelectedTrackCh1;
   delete m_pLoadSelectedTrackCh2;
   delete m_pSelectNextTrack;
   delete m_pSelectPrevTrack;
   delete m_pSelectNextPlaylist;
   delete m_pSelectPrevPlaylist;
   delete m_pLoadSelectedIntoFirstStopped;
   delete m_pSelectTrackKnob;
}

void LibraryControl::bindWidget(WLibrarySidebar* pSidebarWidget, WLibrary* pLibraryWidget, MixxxKeyboard* pKeyboard) {
    if (m_pSidebarWidget != NULL) {
        disconnect(m_pSidebarWidget, 0, this, 0);
    }
    m_pSidebarWidget = pSidebarWidget;
    connect(m_pSidebarWidget, SIGNAL(destroyed()),
            this, SLOT(sidebarWidgetDeleted()));

    if (m_pLibraryWidget != NULL) {
        disconnect(m_pLibraryWidget, 0, this, 0);
    }
    m_pLibraryWidget = pLibraryWidget;
    connect(m_pLibraryWidget, SIGNAL(destroyed()),
            this, SLOT(libraryWidgetDeleted()));
}

void LibraryControl::libraryWidgetDeleted() {
    m_pLibraryWidget = NULL;
}

void LibraryControl::sidebarWidgetDeleted() {
    m_pSidebarWidget = NULL;
}

void LibraryControl::slotLoadSelectedTrackCh1(double v) {
    if (m_pLibraryWidget == NULL)
        return;

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->loadSelectedTrackToGroup("[Channel1]");
    }
}

void LibraryControl::slotLoadSelectedTrackCh2(double v) {
    if (m_pLibraryWidget == NULL)
        return;

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }

        activeView->loadSelectedTrackToGroup("[Channel2]");
    }
}

void LibraryControl::slotLoadSelectedIntoFirstStopped(double v) {
    if (m_pLibraryWidget == NULL)
        return;

    if (v > 0)
    {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->loadSelectedTrack();
    }
}

void LibraryControl::slotSelectNextTrack(double v) {
    if (m_pLibraryWidget == NULL)
        return;
    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->moveSelection(1);
    }
}

void LibraryControl::slotSelectPrevTrack(double v) {
    if (m_pLibraryWidget == NULL)
        return;
    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->moveSelection(-1);
    }
}

void LibraryControl::slotSelectTrackKnob(double v)
{
    if (m_pLibraryWidget == NULL)
        return;

    int i = (int)v;

    LibraryView* activeView = m_pLibraryWidget->getActiveView();
    if (!activeView) {
        return;
    }
    activeView->moveSelection(i);
}


void LibraryControl::slotSelectNextPlaylist(double v) {
    if (m_pSidebarWidget == NULL)
        return;

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

void LibraryControl::slotSelectPrevPlaylist(double v) {
    if (m_pSidebarWidget == NULL)
        return;

    QWidget* destWidget = m_pSidebarWidget;
    if (!destWidget) {
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

