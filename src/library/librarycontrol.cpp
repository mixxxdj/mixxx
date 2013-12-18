#include "library/librarycontrol.h"

#include <QApplication>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QtDebug>

#include "controlobject.h"
#include "controlpushbutton.h"
#include "playermanager.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "library/libraryview.h"
#include "util/container.h"

LoadToGroupController::LoadToGroupController(QObject* pParent, const QString& group)
        : QObject(pParent),
          m_group(group) {
    m_pLoadControl = new ControlPushButton(ConfigKey(group, "LoadSelectedTrack"));
    connect(m_pLoadControl, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadToGroup(double)));

    m_pLoadAndPlayControl = new ControlPushButton(ConfigKey(group, "LoadSelectedTrackAndPlay"));
    connect(m_pLoadAndPlayControl, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadToGroupAndPlay(double)));

    connect(this, SIGNAL(loadToGroup(QString, bool)),
            pParent, SLOT(slotLoadSelectedTrackToGroup(QString, bool)));
}

LoadToGroupController::~LoadToGroupController() {
    delete m_pLoadControl;
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

LibraryControl::LibraryControl(QObject* pParent)
        : QObject(pParent),
          m_pLibraryWidget(NULL),
          m_pSidebarWidget(NULL),
          m_numDecks("[Master]", "num_decks"),
          m_numSamplers("[Master]", "num_samplers"),
          m_numPreviewDecks("[Master]", "num_preview_decks") {

    slotNumDecksChanged(m_numDecks.get());
    slotNumSamplersChanged(m_numSamplers.get());
    slotNumPreviewDecksChanged(m_numPreviewDecks.get());
    connect(&m_numDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumDecksChanged(double)));
    connect(&m_numSamplers, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumSamplersChanged(double)));
    connect(&m_numPreviewDecks, SIGNAL(valueChanged(double)),
            this, SLOT(slotNumPreviewDecksChanged(double)));

    // Make controls for library navigation and track loading. Leaking all these
    // CO's, but oh well?

    m_pSelectNextTrack = new ControlPushButton(ConfigKey("[Playlist]", "SelectNextTrack"));
    connect(m_pSelectNextTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextTrack(double)));

    m_pSelectPrevTrack = new ControlPushButton(ConfigKey("[Playlist]", "SelectPrevTrack"));
    connect(m_pSelectPrevTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevTrack(double)));

    m_pSelectNextPlaylist = new ControlPushButton(ConfigKey("[Playlist]", "SelectNextPlaylist"));
    connect(m_pSelectNextPlaylist, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextSidebarItem(double)));

    m_pSelectPrevPlaylist = new ControlPushButton(ConfigKey("[Playlist]", "SelectPrevPlaylist"));
    connect(m_pSelectPrevPlaylist, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevSidebarItem(double)));

    m_pToggleSidebarItem = new ControlPushButton(ConfigKey("[Playlist]", "ToggleSelectedSidebarItem"));
    connect(m_pToggleSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotToggleSelectedSidebarItem(double)));

    m_pLoadSelectedIntoFirstStopped = new ControlPushButton(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped"));
    connect(m_pLoadSelectedIntoFirstStopped, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadSelectedIntoFirstStopped(double)));

    m_pSelectTrackKnob = new ControlPushButton(ConfigKey("[Playlist]","SelectTrackKnob"));
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
   deleteMapValues(&m_loadToGroupControllers);
}

void LibraryControl::maybeCreateGroupController(const QString& group) {
    LoadToGroupController* pGroup = m_loadToGroupControllers.value(group, NULL);
    if (pGroup == NULL) {
        pGroup = new LoadToGroupController(this, group);
        m_loadToGroupControllers[group] = pGroup;
    }
}

void LibraryControl::slotNumDecksChanged(double v) {
    int iNumDecks = v;

    if (iNumDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForDeck(i));
    }
}

void LibraryControl::slotNumSamplersChanged(double v) {
    int iNumSamplers = v;

    if (iNumSamplers < 0) {
        return;
    }

    for (int i = 0; i < iNumSamplers; ++i) {
        maybeCreateGroupController(PlayerManager::groupForSampler(i));
    }
}

void LibraryControl::slotNumPreviewDecksChanged(double v) {
    int iNumPreviewDecks = v;

    if (iNumPreviewDecks < 0) {
        return;
    }

    for (int i = 0; i < iNumPreviewDecks; ++i) {
        maybeCreateGroupController(PlayerManager::groupForPreviewDeck(i));
    }
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
    if (m_pLibraryWidget == NULL) {
        return;
    }

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->moveSelection(1);
    }
}

void LibraryControl::slotSelectPrevTrack(double v) {
    if (m_pLibraryWidget == NULL) {
        return;
    }

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->moveSelection(-1);
    }
}

void LibraryControl::slotSelectTrackKnob(double v) {
    if (m_pLibraryWidget == NULL) {
        return;
    }

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

