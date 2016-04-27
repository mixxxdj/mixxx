#include "library/librarycontrol.h"

#include <QApplication>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QModelIndexList>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "mixer/playermanager.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "library/library.h"
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

LibraryControl::LibraryControl(Library* pLibrary)
        : QObject(pLibrary),
          m_pLibrary(pLibrary),
          m_pLibraryWidget(NULL),
          m_pSidebarWidget(NULL),
          m_numDecks("[Master]", "num_decks", this),
          m_numSamplers("[Master]", "num_samplers", this),
          m_numPreviewDecks("[Master]", "num_preview_decks", this) {

    slotNumDecksChanged(m_numDecks.get());
    slotNumSamplersChanged(m_numSamplers.get());
    slotNumPreviewDecksChanged(m_numPreviewDecks.get());
    m_numDecks.connectValueChanged(SLOT(slotNumDecksChanged(double)));
    m_numSamplers.connectValueChanged(SLOT(slotNumSamplersChanged(double)));
    m_numPreviewDecks.connectValueChanged(SLOT(slotNumPreviewDecksChanged(double)));

    // Make controls for library navigation and track loading.

    m_pSelectNextTrack = new ControlPushButton(ConfigKey("[Playlist]", "SelectNextTrack"));
    connect(m_pSelectNextTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextTrack(double)));

    m_pSelectPrevTrack = new ControlPushButton(ConfigKey("[Playlist]", "SelectPrevTrack"));
    connect(m_pSelectPrevTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevTrack(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectTrack = new ControlObject(ConfigKey("[Playlist]","SelectTrackKnob"), false);
    connect(m_pSelectTrack, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectTrack(double)));

    m_pSelectNextSidebarItem = new ControlPushButton(ConfigKey("[Playlist]", "SelectNextPlaylist"));
    connect(m_pSelectNextSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectNextSidebarItem(double)));

    m_pSelectPrevSidebarItem = new ControlPushButton(ConfigKey("[Playlist]", "SelectPrevPlaylist"));
    connect(m_pSelectPrevSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectPrevSidebarItem(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pSelectSidebarItem = new ControlObject(ConfigKey("[Playlist]", "SelectPlaylist"), false);
    connect(m_pSelectSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotSelectSidebarItem(double)));

    m_pToggleSidebarItem = new ControlPushButton(ConfigKey("[Playlist]", "ToggleSelectedSidebarItem"));
    connect(m_pToggleSidebarItem, SIGNAL(valueChanged(double)),
            this, SLOT(slotToggleSelectedSidebarItem(double)));

    m_pLoadSelectedIntoFirstStopped = new ControlPushButton(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped"));
    connect(m_pLoadSelectedIntoFirstStopped, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoadSelectedIntoFirstStopped(double)));

    m_pAutoDjAddTop = new ControlPushButton(ConfigKey("[Playlist]","AutoDjAddTop"));
    connect(m_pAutoDjAddTop, SIGNAL(valueChanged(double)),
            this, SLOT(slotAutoDjAddTop(double)));

    m_pAutoDjAddBottom = new ControlPushButton(ConfigKey("[Playlist]","AutoDjAddBottom"));
    connect(m_pAutoDjAddBottom, SIGNAL(valueChanged(double)),
            this, SLOT(slotAutoDjAddBottom(double)));

    // Ignoring no-ops is important since this is for +/- tickers.
    m_pFontSizeKnob = new ControlObject(
            ConfigKey("[Library]", "font_size_knob"), false);
    connect(m_pFontSizeKnob, SIGNAL(valueChanged(double)),
            this, SLOT(slotFontSize(double)));

    m_pFontSizeDecrement = new ControlPushButton(
            ConfigKey("[Library]", "font_size_decrement"));
    connect(m_pFontSizeDecrement, SIGNAL(valueChanged(double)),
            this, SLOT(slotDecrementFontSize(double)));

    m_pFontSizeIncrement = new ControlPushButton(
            ConfigKey("[Library]", "font_size_increment"));
    connect(m_pFontSizeIncrement, SIGNAL(valueChanged(double)),
            this, SLOT(slotIncrementFontSize(double)));
}

LibraryControl::~LibraryControl() {
   delete m_pSelectNextTrack;
   delete m_pSelectPrevTrack;
   delete m_pSelectTrack;
   delete m_pSelectNextSidebarItem;
   delete m_pSelectPrevSidebarItem;
   delete m_pSelectSidebarItem;
   delete m_pToggleSidebarItem;
   delete m_pLoadSelectedIntoFirstStopped;
   delete m_pAutoDjAddTop;
   delete m_pAutoDjAddBottom;
   delete m_pFontSizeKnob;
   delete m_pFontSizeDecrement;
   delete m_pFontSizeIncrement;
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

void LibraryControl::bindWidget(WLibrary* pLibraryWidget, KeyboardEventFilter* pKeyboard) {
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

void LibraryControl::slotAutoDjAddTop(double v) {
    if (m_pLibraryWidget == NULL) {
        return;
    }

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->slotSendToAutoDJTop();
    }
}

void LibraryControl::slotAutoDjAddBottom(double v) {
    if (m_pLibraryWidget == NULL) {
        return;
    }

    if (v > 0) {
        LibraryView* activeView = m_pLibraryWidget->getActiveView();
        if (!activeView) {
            return;
        }
        activeView->slotSendToAutoDJ();
    }
}

void LibraryControl::slotSelectNextTrack(double v) {
    if (v > 0) {
        slotSelectTrack(1);
    }
}

void LibraryControl::slotSelectPrevTrack(double v) {
    if (v > 0) {
        slotSelectTrack(-1);
    }
}

void LibraryControl::slotSelectTrack(double v) {
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

void LibraryControl::slotSelectSidebarItem(double v) {
    if (m_pSidebarWidget == NULL) {
        return;
    }
    if (v > 0) {
        QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
            QEvent::KeyPress,
            (int)Qt::Key_Down, Qt::NoModifier, QString(), true));
        QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
            QEvent::KeyRelease,
            (int)Qt::Key_Down, Qt::NoModifier, QString(), true));
    } else if (v < 0) {
        QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
            QEvent::KeyPress,
            (int)Qt::Key_Up, Qt::NoModifier, QString(), true));
        QApplication::postEvent(m_pSidebarWidget, new QKeyEvent(
            QEvent::KeyRelease,
            (int)Qt::Key_Up, Qt::NoModifier, QString(), true));
    }
}

void LibraryControl::slotSelectNextSidebarItem(double v) {
    if (v > 0) {
        slotSelectSidebarItem(1);
    }
}

void LibraryControl::slotSelectPrevSidebarItem(double v) {
    if (v > 0) {
        slotSelectSidebarItem(-1);
    }
}

void LibraryControl::slotToggleSelectedSidebarItem(double v) {
    if (m_pSidebarWidget != NULL && v > 0) {
        m_pSidebarWidget->toggleSelectedItem();
    }
}

void LibraryControl::slotFontSize(double v) {
    if (v == 0.0) {
        return;
    }
    QFont font = m_pLibrary->getTrackTableFont();
    font.setPointSizeF(font.pointSizeF() + v);
    m_pLibrary->slotSetTrackTableFont(font);
}

void LibraryControl::slotIncrementFontSize(double v) {
    if (v > 0.0) {
        slotFontSize(1);
    }
}

void LibraryControl::slotDecrementFontSize(double v) {
    if (v > 0.0) {
        slotFontSize(-1);
    }
}
