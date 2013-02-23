#include <QDebug>
#include <QModelIndexList>
#include <QModelIndex>
#include <QItemSelectionModel>

#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "playermanager.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "library/librarycontrol.h"
#include "library/libraryview.h"

LoadToGroupController::LoadToGroupController(QObject* pParent,
                                             const QString group) :
        QObject(pParent),
        m_group(group) {
    m_pLoadControl = new ControlObject(ConfigKey(group, "LoadSelectedTrack"));
    m_pLoadCOTM = new ControlObjectThreadMain(m_pLoadControl);
    connect(m_pLoadCOTM, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadToGroup(double)));

    m_pLoadAndPlayControl = new ControlObject(ConfigKey(group, "LoadSelectedTrackAndPlay"));
    m_pLoadAndPlayCOTM = new ControlObjectThreadMain(m_pLoadAndPlayControl);
    connect(m_pLoadAndPlayCOTM, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadToGroupAndPlay(double)));

    connect(this, SIGNAL(loadToGroup(QString, bool)),
            pParent, SLOT(slotLoadSelectedTrackToGroup(QString, bool)));
}

LoadToGroupController::~LoadToGroupController() {
    delete m_pLoadCOTM;
    delete m_pLoadControl;
    delete m_pLoadAndPlayCOTM;
    delete m_pLoadAndPlayControl;
}

void LoadToGroupController::slotLoadToGroup(double v) {
    if (v > 0) {
        emit(loadToGroup(m_group, false));
    }
}

void LoadToGroupController::slotLoadToGroupAndPlay(double v) {
    if (v > 0) {
        emit(loadToGroup(m_group, true));
    }
}


LibraryControl::LibraryControl(QObject* pParent) : QObject(pParent) {
    m_pLibraryWidget = NULL;
    m_pSidebarWidget = NULL;

    // Make controls for library navigation and track loading. Leaking all these CO's, but oh well?

    // We don't know how many decks or samplers there will be, so create 10 of them.
    //TODO(xxx) find out the correct numbers
    for (int i = 0; i < 10; ++i) {
        (void)new LoadToGroupController(
                this, PlayerManager::groupForDeck(i));
        (void)new LoadToGroupController(
                this, PlayerManager::groupForSampler(i));
        (void)new LoadToGroupController(
                this, PlayerManager::groupForPreviewDeck(i));
    }

    m_pSelectNextTrack = new ControlObjectThreadMain(
        new ControlObject(ConfigKey("[Playlist]", "SelectNextTrack")));
    connect(m_pSelectNextTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextTrack(double)));

    m_pSelectPrevTrack = new ControlObjectThreadMain(
        new ControlObject(ConfigKey("[Playlist]", "SelectPrevTrack")));
    connect(m_pSelectPrevTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevTrack(double)));

    m_pSelectNextPlaylist = new ControlObjectThreadMain(
        new ControlObject(ConfigKey("[Playlist]", "SelectNextPlaylist")));
    connect(m_pSelectNextPlaylist, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextSidebarItem(double)));

    m_pSelectPrevPlaylist = new ControlObjectThreadMain(
        new ControlObject(ConfigKey("[Playlist]", "SelectPrevPlaylist")));
    connect(m_pSelectPrevPlaylist, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevSidebarItem(double)));

    m_pToggleSidebarItem = new ControlObjectThreadMain(
        new ControlObject(ConfigKey("[Playlist]", "ToggleSelectedSidebarItem")));
    connect(m_pToggleSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotToggleSelectedSidebarItem(double)));

    m_pLoadSelectedIntoFirstStopped = new ControlObjectThreadMain(
        new ControlObject(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped")));
    connect(m_pLoadSelectedIntoFirstStopped, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadSelectedIntoFirstStopped(double)));

    m_pSelectTrackKnob = new ControlObjectThreadMain(
        new ControlObject(ConfigKey("[Playlist]","SelectTrackKnob")));
    connect(m_pSelectTrackKnob, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectTrackKnob(double)));
}

LibraryControl::~LibraryControl() {
   delete m_pSelectNextTrack;
   delete m_pSelectPrevTrack;
   delete m_pSelectNextPlaylist;
   delete m_pSelectPrevPlaylist;
   delete m_pToggleSidebarItem;
   delete m_pLoadSelectedIntoFirstStopped;
   delete m_pSelectTrackKnob;
}

void LibraryControl::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    if (m_pSidebarWidget != NULL) {
        disconnect(m_pSidebarWidget, 0, this, 0);
    }
    m_pSidebarWidget = pSidebarWidget;
    connect(m_pSidebarWidget, SIGNAL(destroyed()),
            this, SLOT(sidebarWidgetDeleted()));
}

void LibraryControl::bindWidget(WLibrary* pLibraryWidget, MixxxKeyboard* pKeyboard) {
    Q_UNUSED(pKeyboard);
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

void LibraryControl::slotLoadSelectedTrackToGroup(QString group, bool play) {
    if (m_pLibraryWidget == NULL) {
        return;
    }

    LibraryView* activeView = m_pLibraryWidget->getActiveView();
    if (!activeView) {
        return;
    }
    activeView->loadSelectedTrackToGroup(group, play);
}

void LibraryControl::slotLoadSelectedIntoFirstStopped(double v) {
    if (m_pLibraryWidget == NULL) {
        return;
    }

    if (v > 0) {
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

void LibraryControl::slotSelectNextSidebarItem(double v) {
    if (m_pSidebarWidget == NULL) {
        return;
    }
    QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
        v > 0 ? QEvent::KeyPress : QEvent::KeyRelease,
        (int)Qt::Key_Down, Qt::NoModifier, QString(), true));
}

void LibraryControl::slotSelectPrevSidebarItem(double v) {
    if (m_pSidebarWidget == NULL) {
        return;
    }
    QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
        v > 0 ? QEvent::KeyPress : QEvent::KeyRelease,
        (int)Qt::Key_Up, Qt::NoModifier, QString(), true));
}

void LibraryControl::slotToggleSelectedSidebarItem(double v) {
    if (m_pSidebarWidget == NULL) {
        return;
    }
    QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
        v > 0 ? QEvent::KeyPress : QEvent::KeyRelease,
        (int)Qt::Key_Return, Qt::NoModifier, QString(), true));
}

